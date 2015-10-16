/*
==============================================================================

This file is part of the JUCE library.
Copyright (c) 2013 - Raw Material Software Ltd.

Permission is granted to use this software under the terms of either:
a) the GPL v2 (or any later version)
b) the Affero GPL v3

Details of these licenses can be found at: www.gnu.org/licenses

JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

------------------------------------------------------------------------------

To release a closed-source product which uses JUCE, commercial licenses are
available: visit www.juce.com for more information.

==============================================================================
*/
#include "NewAudioProcessorGraph.h"

const int NewAudioProcessorGraph::midiChannelIndex = 0x1000;

//==============================================================================
namespace GraphRenderingOps
{

//==============================================================================
class AudioGraphRenderingOp
{
public:
	AudioGraphRenderingOp() {}
	virtual ~AudioGraphRenderingOp() {}

	virtual void perform(AudioSampleBuffer& sharedBufferChans,
		const OwnedArray <MidiBuffer>& sharedMidiBuffers,
		const int numSamples) = 0;

	JUCE_LEAK_DETECTOR(AudioGraphRenderingOp)
};

//==============================================================================
class ClearChannelOp : public AudioGraphRenderingOp
{
public:
	explicit ClearChannelOp(const int channelNum_)
		: channelNum(channelNum_)
	{
	}

	void perform(AudioSampleBuffer& sharedBufferChans, const OwnedArray <MidiBuffer>&, const int numSamples) override
	{
		sharedBufferChans.clear(channelNum, 0, numSamples);
	}

private:
	const int channelNum;

	JUCE_DECLARE_NON_COPYABLE(ClearChannelOp)
};

//==============================================================================
class CopyChannelOp : public AudioGraphRenderingOp
{
public:
	CopyChannelOp(const int srcChannelNum_, const int dstChannelNum_)
		: srcChannelNum(srcChannelNum_),
		dstChannelNum(dstChannelNum_)
	{
	}

	void perform(AudioSampleBuffer& sharedBufferChans, const OwnedArray <MidiBuffer>&, const int numSamples) override
	{
		sharedBufferChans.copyFrom(dstChannelNum, 0, sharedBufferChans, srcChannelNum, 0, numSamples);
	}

private:
	const int srcChannelNum, dstChannelNum;

	JUCE_DECLARE_NON_COPYABLE(CopyChannelOp)
};

//==============================================================================
class AddChannelOp : public AudioGraphRenderingOp
{
public:
	AddChannelOp(const int srcChannelNum_, const int dstChannelNum_)
		: srcChannelNum(srcChannelNum_),
		dstChannelNum(dstChannelNum_)
	{
	}

	void perform(AudioSampleBuffer& sharedBufferChans, const OwnedArray <MidiBuffer>&, const int numSamples) override
	{
		sharedBufferChans.addFrom(dstChannelNum, 0, sharedBufferChans, srcChannelNum, 0, numSamples);
	}

private:
	const int srcChannelNum, dstChannelNum;

	JUCE_DECLARE_NON_COPYABLE(AddChannelOp)
};

//==============================================================================
class ClearMidiBufferOp : public AudioGraphRenderingOp
{
public:
	ClearMidiBufferOp(const int bufferNum_)
		: bufferNum(bufferNum_)
	{
	}

	void perform(AudioSampleBuffer&, const OwnedArray <MidiBuffer>& sharedMidiBuffers, const int) override
	{
		sharedMidiBuffers.getUnchecked(bufferNum)->clear();
	}

private:
	const int bufferNum;

	JUCE_DECLARE_NON_COPYABLE(ClearMidiBufferOp)
};

//==============================================================================
class CopyMidiBufferOp : public AudioGraphRenderingOp
{
public:
	CopyMidiBufferOp(const int srcBufferNum_, const int dstBufferNum_)
		: srcBufferNum(srcBufferNum_),
		dstBufferNum(dstBufferNum_)
	{
	}

	void perform(AudioSampleBuffer&, const OwnedArray <MidiBuffer>& sharedMidiBuffers, const int)
	{
		*sharedMidiBuffers.getUnchecked(dstBufferNum) = *sharedMidiBuffers.getUnchecked(srcBufferNum);
	}

private:
	const int srcBufferNum, dstBufferNum;

	JUCE_DECLARE_NON_COPYABLE(CopyMidiBufferOp)
};

//==============================================================================
class AddMidiBufferOp : public AudioGraphRenderingOp
{
public:
	AddMidiBufferOp(const int srcBufferNum_, const int dstBufferNum_)
		: srcBufferNum(srcBufferNum_),
		dstBufferNum(dstBufferNum_)
	{
	}

	void perform(AudioSampleBuffer&, const OwnedArray <MidiBuffer>& sharedMidiBuffers, const int numSamples)
	{
		sharedMidiBuffers.getUnchecked(dstBufferNum)
			->addEvents(*sharedMidiBuffers.getUnchecked(srcBufferNum), 0, numSamples, 0);
	}

private:
	const int srcBufferNum, dstBufferNum;

	JUCE_DECLARE_NON_COPYABLE(AddMidiBufferOp)
};

//==============================================================================
class DelayChannelOp : public AudioGraphRenderingOp
{
public:
	DelayChannelOp(const int channel_, const int numSamplesDelay_)
		: channel(channel_),
		bufferSize(numSamplesDelay_ + 1),
		readIndex(0), writeIndex(numSamplesDelay_)
	{
		buffer.calloc((size_t)bufferSize);
	}

	void perform(AudioSampleBuffer& sharedBufferChans, const OwnedArray <MidiBuffer>&, const int numSamples)
	{
		float* data = sharedBufferChans.getWritePointer(channel, 0);

		for (int i = numSamples; --i >= 0;)
		{
			buffer[writeIndex] = *data;
			*data++ = buffer[readIndex];

			if (++readIndex >= bufferSize) readIndex = 0;
			if (++writeIndex >= bufferSize) writeIndex = 0;
		}
	}

private:
	HeapBlock<float> buffer;
	const int channel, bufferSize;
	int readIndex, writeIndex;

	JUCE_DECLARE_NON_COPYABLE(DelayChannelOp)
};


//==============================================================================
class ProcessBufferOp : public AudioGraphRenderingOp
{
public:
	ProcessBufferOp(const NewAudioProcessorGraph::Node::Ptr& node_,
		const Array <int>& audioChannelsToUse_,
		const int totalChans_,
		const int midiBufferToUse_)
		: node(node_),
		processor(node_->getProcessor()),
		audioChannelsToUse(audioChannelsToUse_),
		totalChans(jmax(1, totalChans_)),
		midiBufferToUse(midiBufferToUse_)
	{
		channels.calloc((size_t)totalChans);

		while (audioChannelsToUse.size() < totalChans)
			audioChannelsToUse.add(0);
	}

	void perform(AudioSampleBuffer& sharedBufferChans, const OwnedArray <MidiBuffer>& sharedMidiBuffers, const int numSamples)
	{
		for (int i = totalChans; --i >= 0;)
			channels[i] = sharedBufferChans.getWritePointer(audioChannelsToUse.getUnchecked(i), 0);

		AudioSampleBuffer buffer(channels, totalChans, numSamples);

		processor->processBlock(buffer, *sharedMidiBuffers.getUnchecked(midiBufferToUse));
	}

	const NewAudioProcessorGraph::Node::Ptr node;
	AudioProcessor* const processor;

private:
	Array<int> audioChannelsToUse;
	HeapBlock<float*> channels;
	int totalChans;
	int midiBufferToUse;

	JUCE_DECLARE_NON_COPYABLE(ProcessBufferOp)
};


class MapNode;

/**
* Represents a connection between two MapNodes.  Both the source and destination node will have their own copy of this information.
*/
struct MapNodeConnection
{
	MapNodeConnection(MapNode* srcMapNode, MapNode* dstMapNode, uint32 srcChannelIndex, uint32 dstChannelIndex) noexcept :
	sourceMapNode(srcMapNode), destMapNode(dstMapNode), sourceChannelIndex(srcChannelIndex), destChannelIndex(dstChannelIndex)
	{
	}

	MapNode* sourceMapNode;
	MapNode* destMapNode;
	const uint32 sourceChannelIndex;
	const uint32 destChannelIndex;

	JUCE_LEAK_DETECTOR(MapNodeConnection);
};

/**
* Wraps an NewAudioProcessorGraph::Node providing information regarding source (input) and destination (output) connections, input latencies and sorting.
*/
class MapNode
{
public:

	MapNode(const int nodeId, NewAudioProcessorGraph::Node* node) noexcept :
		feedbackLoopCheck(0), nodeId(nodeId), node(node), renderIndex(0), maxInputLatency(0), maxLatency(0)
	{
	}

	uint32 getNodeId() const noexcept
	{
		return nodeId;
	}

	uint32 getRenderIndex() const noexcept
	{
		return renderIndex;
	}

	NewAudioProcessorGraph::Node* getNode() const noexcept
	{
		return node;
	}

	void setRenderIndex(uint32 theRenderIndex)
	{
		renderIndex = theRenderIndex;
	}

	int getMaxInputLatency() const noexcept
	{
		return maxInputLatency;
	}

	int getMaxLatency() const noexcept
	{
		return maxLatency;
	}

	const Array<MapNode*>& getUniqueSources() const noexcept
	{
		return uniqueSources;
	}

	const Array<MapNode*>& getUniqueDestinations() const noexcept
	{
		return uniqueDestinations;
	}

	void addInputConnection(MapNode* srcMapNode, MapNode* destMapNode, uint32 sourceChannelIndex, uint32 destChannelIndex)
	{
		sourceConnections.add(new MapNodeConnection(srcMapNode, destMapNode, sourceChannelIndex, destChannelIndex));
	}

	void addOutputConnection(MapNode* srcMapNode, MapNode* destMapNode, uint32 sourceChannelIndex, uint32 destChannelIndex)
	{
		destConnections.add(new MapNodeConnection(srcMapNode, destMapNode, sourceChannelIndex, destChannelIndex));
	}

	Array<MapNodeConnection*> getSourcesToChannel(uint32 channel) const
	{
		Array<MapNodeConnection*> sourcesToChannel;

		for (int i = 0; i < sourceConnections.size(); ++i)
		{
			MapNodeConnection* ec = sourceConnections.getUnchecked(i);

			if (ec->destChannelIndex == channel)
				sourcesToChannel.add(ec);
		}

		return sourcesToChannel;
	}

	const OwnedArray<MapNodeConnection>& getSourceConnections() const noexcept
	{
		return sourceConnections;
	}

	const OwnedArray<MapNodeConnection>& getDestConnections() const noexcept
	{
		return destConnections;
	}

	void calculateLatenciesWithInputLatency(int inputLatency)
	{
		maxInputLatency = inputLatency;
		maxLatency = maxInputLatency + node->getProcessor()->getLatencySamples();
	}

	void cacheUniqueNodeConnections()
	{
		//This should only ever be called once but in case things change
		uniqueSources.clear();
		uniqueDestinations.clear();

		for (int i = 0; i < sourceConnections.size(); ++i)
		{
			const MapNodeConnection* ec = sourceConnections.getUnchecked(i);

			if (!uniqueSources.contains(ec->sourceMapNode))
				uniqueSources.add(ec->sourceMapNode);
		}

		for (int i = 0; i < destConnections.size(); ++i)
		{
			const MapNodeConnection* ec = destConnections.getUnchecked(i);

			if (!uniqueDestinations.contains(ec->destMapNode))
				uniqueDestinations.add(ec->destMapNode);
		}

	}

	uint32 feedbackLoopCheck;

private:

	const int nodeId;
	NewAudioProcessorGraph::Node* node;
	uint32 renderIndex;
	int maxInputLatency;
	int maxLatency;

	OwnedArray<MapNodeConnection> sourceConnections;
	OwnedArray<MapNodeConnection> destConnections;
	Array<MapNode*> uniqueSources;
	Array<MapNode*> uniqueDestinations;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MapNode);

};

/**
* Combines Node and Connection data into MapNodes and sorts them.  Sorted MapNodes are provided to the RenderingOpsSequenceCalculator
*/
class GraphMap
{
public:

	GraphMap() {};

	void buildMap(const OwnedArray<NewAudioProcessorGraph::Connection>& connections, const ReferenceCountedArray<NewAudioProcessorGraph::Node>& nodes)
	{
		mapNodes.ensureStorageAllocated(nodes.size());

		//Create MapNode for every node.
		for (int i = 0; i < nodes.size(); ++i)
		{
			NewAudioProcessorGraph::Node* node = nodes.getUnchecked(i);

			//#smell - just want to get the insert index, we don't care about the returned node.
			int index;
			MapNode* foundMapNode = findMapNode(node->nodeId, index);
			jassert(foundMapNode == nullptr);  //cannot have duplicate nodeIds.

			mapNodes.insert(index, new MapNode(node->nodeId, node));

		}

		//Add MapNodeConnections to MapNodes
		for (int i = 0; i < connections.size(); ++i)
		{
			const NewAudioProcessorGraph::Connection* c = connections.getUnchecked(i);

			//#smell - We don't care about index here but we do care about the returned node.
			int index;
			MapNode* srcMapNode = findMapNode(c->sourceNodeId, index);
			MapNode* destMapNode = findMapNode(c->destNodeId, index);

			jassert(srcMapNode != nullptr);

			if (srcMapNode != nullptr)
			{
				srcMapNode->addOutputConnection(srcMapNode, destMapNode, c->sourceChannelIndex, c->destChannelIndex);
				destMapNode->addInputConnection(srcMapNode, destMapNode, c->sourceChannelIndex, c->destChannelIndex);
			}

		}

		//MapNodes have all their connections now, but for sorting we only care about connections to distinct nodes so
		//cache lists of the distinct source and destination nodes.
		for (int i = 0; i < mapNodes.size(); ++i)
			mapNodes.getUnchecked(i)->cacheUniqueNodeConnections();


		//Grab all the nodes that have no input connections (they are used as the starting points for the sort routine)
		Array<MapNode*> nodesToSort;
		for (int i = 0; i < mapNodes.size(); ++i)
		{
			MapNode* mapNode = mapNodes.getUnchecked(i);

			if (mapNode->getUniqueSources().size() == 0)
				nodesToSort.add(mapNode);
		}

		sortedMapNodes.ensureStorageAllocated(mapNodes.size());

		//Sort the nodes
		while (true)
		{
			Array<MapNode*> nextNodes;
			this->sortNodes(nodesToSort, sortedMapNodes, nextNodes);

			if (nextNodes.size() == 0)
				break;

			nodesToSort.clear();
			nodesToSort.addArray(nextNodes);

		}

	}

	const Array<MapNode*>& getSortedMapNodes() const noexcept
	{
		return sortedMapNodes;
	}

private:

	MapNode* findMapNode(const int destNodeId, int& insertIndex) const noexcept
	{

		int start = 0;
		int end = mapNodes.size();

		for (;;)
		{
			if (start >= end)
			{
				break;
			} else if (destNodeId == mapNodes.getUnchecked(start)->getNodeId())
			{
				insertIndex = start;
				return mapNodes.getUnchecked(start);
				break;
			} else
			{
				const int halfway = (start + end) / 2;

				if (halfway == start)
				{
					if (destNodeId >= mapNodes.getUnchecked(halfway)->getNodeId())
						++start;

					break;
				} else if (destNodeId >= mapNodes.getUnchecked(halfway)->getNodeId())
					start = halfway;
				else
					end = halfway;
			}
		}

		insertIndex = start;

		return nullptr;

	}

	/*
	* Iterates the given startNodes and if a node's sources have all been sorted then the node is inserted into the sort list.  If the node is still waiting
	* for a source to be sorted it is added to the nextNodes list for the next call.  If a node is sorted then it's destination nodes are added to the nextNodes list.
	*/
	void sortNodes(Array<MapNode*>& startNodes, Array<MapNode*>& sortedNodes, Array<MapNode*>& nextNodes) const
	{

		for (int i = 0; i < startNodes.size(); ++i)
		{
			MapNode* node = startNodes.getUnchecked(i);

			const Array<MapNode*>& uniqueSources = node->getUniqueSources();

			bool canBeSorted = true;

			if (uniqueSources.size() > 1)
			{
				for (int j = 0; j < uniqueSources.size(); ++j)
				{
					MapNode* sourceNode = uniqueSources.getUnchecked(j);

					if (!isNodeSorted(sourceNode))
					{
						sourceNode->feedbackLoopCheck++;

						//Less than fantastic way to deal with feedback loops but at least it eliminates an infinite loop.
						//5000 attempts waiting for this sourceNode to be sorted probably means there's a feedback loop.  Let the node proceed if so.
						canBeSorted = sourceNode->feedbackLoopCheck > 5000;

						jassert(sourceNode->feedbackLoopCheck <= 5000); //Feedback loop detected.  Feedback loops are not supported, will not work, and sometimes mess up the sorting.

						if (!canBeSorted)
							break;

					}

				}

			}

			if (canBeSorted)
			{
				//Determine the latency information for this node.
				int maxInputLatency = 0;

				for (int j = 0; j < uniqueSources.size(); ++j)
					maxInputLatency = jmax(maxInputLatency, uniqueSources.getUnchecked(j)->getMaxLatency());

				node->calculateLatenciesWithInputLatency(maxInputLatency);

				//Add the node to the sorted list
				sortedNodes.add(node);
				node->setRenderIndex(sortedNodes.size());

				//in some graph configurations the node we just sorted could already have been inserted in the next nodes list.
				nextNodes.removeFirstMatchingValue(node);

				//Add unsorted destination nodes to the next round of sorting.
				const Array<MapNode*>& uniqueDestinations = node->getUniqueDestinations();

				for (int j = 0; j < uniqueDestinations.size(); ++j)
				{
					MapNode* nextNode = uniqueDestinations.getUnchecked(j);

					if (!nextNodes.contains(nextNode) && !isNodeSorted(nextNode))
						nextNodes.add(nextNode);
				}

			} else
			{
				nextNodes.add(node);
			}

		}

	}

	bool isNodeSorted(const MapNode* mapNode) const
	{
		return mapNode->getRenderIndex() > 0;
	}

	OwnedArray<MapNode> mapNodes;
	Array<MapNode*> sortedMapNodes;

	JUCE_DECLARE_NON_COPYABLE(GraphMap);

};


//==============================================================================
/** Used to calculate the correct sequence of rendering ops needed, based on
the best re-use of shared buffers at each stage.
*/
class RenderingOpSequenceCalculator
{
public:
	//==============================================================================
	RenderingOpSequenceCalculator(NewAudioProcessorGraph& graph_,
		const ReferenceCountedArray<NewAudioProcessorGraph::Node>& nodes_,
		const OwnedArray<NewAudioProcessorGraph::Connection>& connections_,
		Array<void*>& renderingOps)
		: graph(graph_),
		totalLatency(0)
	{
		audioChannelBuffers.add(new ChannelBufferInfo(0, (uint32)zeroNodeID, 0, 0));
		midiChannelBuffers.add(new ChannelBufferInfo(0, (uint32)zeroNodeID, 0, 0));

		GraphRenderingOps::GraphMap graphMap;
		graphMap.buildMap(connections_, nodes_);

		for (int i = 0; i < graphMap.getSortedMapNodes().size(); ++i)
		{
			const MapNode* mapNode = graphMap.getSortedMapNodes().getUnchecked(i);
			createRenderingOpsForNode(mapNode, renderingOps);
			markAnyUnusedBuffersAsFree(mapNode);
		}

		graph.setLatencySamples(totalLatency);

	}

	int getNumBuffersNeeded() const { return audioChannelBuffers.size(); }
	int getNumMidiBuffersNeeded() const { return midiChannelBuffers.size(); }

private:

	/**
	* A list of these are built up as needed by each call to the createRenderingOpsForNode method.  For each call the list is updated
	* by adding more to the list or marking existing ChannelBufferInfo's as free for use on the next pass.  When a ChannelBufferInfo is assigned to a node's output
	* channel, the connection data is analysed to determine at what point the channel will be free for use again.
	*/
	struct ChannelBufferInfo
	{
		//#smell A bit undesireable having two constructors for this but need to allow for freeNodeID and zeroNodeID which don't have corresponding MapNodes.  Perhaps
		//there's a better approach.
		ChannelBufferInfo(uint32 bufferIndex, uint32 nID, uint32 chanIndex, uint32 freeAtNodeRenderIndex) :
			index(bufferIndex), nodeId(nID), channelIndex(chanIndex), freeAtRenderIndex(freeAtNodeRenderIndex), mapNode(nullptr)
		{
		}

		ChannelBufferInfo(uint32 bufferIndex, const MapNode* mapNode, uint32 chanIndex, uint32 freeAtNodeRenderIndex) :
			index(bufferIndex), nodeId(mapNode->getNodeId()), channelIndex(chanIndex), freeAtRenderIndex(freeAtNodeRenderIndex), mapNode(mapNode)
		{
		}

		const uint32 index, nodeId, channelIndex, freeAtRenderIndex;
		const MapNode* mapNode;

	};

	//==============================================================================
	NewAudioProcessorGraph& graph;
	OwnedArray<ChannelBufferInfo> audioChannelBuffers;
	OwnedArray <ChannelBufferInfo> midiChannelBuffers;

	enum { freeNodeID = 0xffffffff, zeroNodeID = 0xfffffffe };

	static bool isNodeBusy(uint32 nodeID) noexcept { return nodeID != freeNodeID && nodeID != zeroNodeID; }

	int totalLatency;

	//==============================================================================
	void createRenderingOpsForNode(const MapNode* mapNode, Array<void*>& renderingOps)
	{
		const int numIns = mapNode->getNode()->getProcessor()->getNumInputChannels();
		const int numOuts = mapNode->getNode()->getProcessor()->getNumOutputChannels();
		const int totalChans = jmax(numIns, numOuts);

		Array <int> audioChannelsToUse;

		const int maxInputLatency = mapNode->getMaxInputLatency();

		for (int inputChan = 0; inputChan < numIns; ++inputChan)
		{
			const ChannelBufferInfo* audioChannelBufferToUse = nullptr;

			const Array<GraphRenderingOps::MapNodeConnection*> srcConnectionsToChannel = mapNode->getSourcesToChannel(inputChan);

			if (srcConnectionsToChannel.size() == 0)
			{
				// unconnected input channel

				if (inputChan >= numOuts)
				{
					audioChannelBufferToUse = getReadOnlyEmptyBuffer();
				} else
				{
					audioChannelBufferToUse = getFreeBuffer(false);
					renderingOps.add(new ClearChannelOp(audioChannelBufferToUse->index));
				}
			} else if (srcConnectionsToChannel.size() == 1)
			{
				// channel with a straightforward single input..
				MapNodeConnection* sourceConnection = srcConnectionsToChannel.getUnchecked(0);

				//get the buffer index for the src node's channel
				audioChannelBufferToUse = getBufferContaining(sourceConnection->sourceMapNode->getNodeId(), sourceConnection->sourceChannelIndex);

				if (audioChannelBufferToUse == nullptr)
				{
					// if not found, this is probably a feedback loop
					audioChannelBufferToUse = getReadOnlyEmptyBuffer();
				}

				if (inputChan < numOuts
					&& isBufferNeededLater(audioChannelBufferToUse, mapNode, inputChan))
				{
					// can't mess up this channel because it's needed later by another node, so we
					// need to use a copy of it..
					const ChannelBufferInfo* newFreeBuffer = getFreeBuffer(false);

					renderingOps.add(new CopyChannelOp(audioChannelBufferToUse->index, newFreeBuffer->index));

					audioChannelBufferToUse = newFreeBuffer;
				}

				const int connectionInputLatency = sourceConnection->sourceMapNode->getMaxLatency();

				if (connectionInputLatency < maxInputLatency)
					renderingOps.add(new DelayChannelOp(audioChannelBufferToUse->index, maxInputLatency - connectionInputLatency));
			} else
			{
				// channel with a mix of several inputs..

				// try to find a re-usable channel from our inputs..
				int reusableInputIndex = -1;

				for (int i = 0; i < srcConnectionsToChannel.size(); ++i)
				{
					MapNodeConnection* mapNodeConnection = srcConnectionsToChannel.getUnchecked(i);

					const uint32 sourceNodeId = mapNodeConnection->sourceMapNode->getNodeId();
					const uint32 sourceChannelIndex = mapNodeConnection->sourceChannelIndex;

					const ChannelBufferInfo* sourceBuffer = getBufferContaining(sourceNodeId, sourceChannelIndex);

					//this seems to hit if a feedback loop was detected in the sorting routine
					jassert(sourceBuffer != nullptr);

					if (sourceBuffer != nullptr
						&& !isBufferNeededLater(sourceBuffer, mapNode, inputChan))
					{
						// we've found one of our input chans that can be re-used..
						reusableInputIndex = i;
						audioChannelBufferToUse = sourceBuffer;

						const int connectionInputLatency = mapNodeConnection->sourceMapNode->getMaxLatency();

						if (connectionInputLatency < maxInputLatency)
							renderingOps.add(new DelayChannelOp(sourceBuffer->index, maxInputLatency - connectionInputLatency));

						break;
					}
				}

				if (reusableInputIndex < 0)
				{
					// can't re-use any of our input chans, so get a new one and copy everything into it..
					audioChannelBufferToUse = getFreeBuffer(false);

					MapNodeConnection* sourceConnection = srcConnectionsToChannel.getUnchecked(0);

					const uint32 firstSourceNodeId = sourceConnection->sourceMapNode->getNodeId();

					const ChannelBufferInfo* sourceBuffer = getBufferContaining(firstSourceNodeId,
						srcConnectionsToChannel.getUnchecked(0)->sourceChannelIndex);
					if (sourceBuffer == nullptr)
					{
						// if not found, this is probably a feedback loop
						renderingOps.add(new ClearChannelOp(audioChannelBufferToUse->index));
					} else
					{
						renderingOps.add(new CopyChannelOp(sourceBuffer->index, audioChannelBufferToUse->index));
					}

					reusableInputIndex = 0;
					const int connectionInputLatency = sourceConnection->sourceMapNode->getMaxLatency();

					if (connectionInputLatency < maxInputLatency)
						renderingOps.add(new DelayChannelOp(audioChannelBufferToUse->index, maxInputLatency - connectionInputLatency));
				}

				for (int j = 0; j < srcConnectionsToChannel.size(); ++j)
				{
					GraphRenderingOps::MapNodeConnection* mapNodeConnection = srcConnectionsToChannel.getUnchecked(j);

					const uint32 sourceNodeId = mapNodeConnection->sourceMapNode->getNodeId();
					const uint32 sourceChannelIndex = mapNodeConnection->sourceChannelIndex;

					if (j != reusableInputIndex)
					{
						const ChannelBufferInfo* sourceBuffer = getBufferContaining(sourceNodeId,
							sourceChannelIndex);
						if (sourceBuffer != nullptr)
						{
							const int connectionInputLatency = mapNodeConnection->sourceMapNode->getMaxLatency();

							if (connectionInputLatency < maxInputLatency)
							{
								if (!isBufferNeededLater(sourceBuffer, mapNode, inputChan))
								{
									renderingOps.add(new DelayChannelOp(sourceBuffer->index, maxInputLatency - connectionInputLatency));
								} else // buffer is reused elsewhere, can't be delayed
								{
									const ChannelBufferInfo* bufferToDelay = getFreeBuffer(false);
									renderingOps.add(new CopyChannelOp(sourceBuffer->index, bufferToDelay->index));
									renderingOps.add(new DelayChannelOp(bufferToDelay->index, maxInputLatency - connectionInputLatency));
									sourceBuffer = bufferToDelay;
								}
							}

							renderingOps.add(new AddChannelOp(sourceBuffer->index, audioChannelBufferToUse->index));
						}
					}
				}
			}

			jassert(audioChannelBufferToUse != nullptr);
			audioChannelsToUse.add(audioChannelBufferToUse->index);

			if (inputChan < numOuts)
				markBufferAsContaining(audioChannelBufferToUse->index, mapNode, inputChan);
		}

		for (int outputChan = numIns; outputChan < numOuts; ++outputChan)
		{
			const ChannelBufferInfo* freeBuffer = getFreeBuffer(false);
			audioChannelsToUse.add(freeBuffer->index);
			markBufferAsContaining(freeBuffer->index, mapNode, outputChan);
		}

		// Now the same thing for midi..

		const ChannelBufferInfo* midiBufferToUse = nullptr;

		const Array<GraphRenderingOps::MapNodeConnection*> midiSourceConnections = mapNode->getSourcesToChannel(NewAudioProcessorGraph::midiChannelIndex);

		if (midiSourceConnections.size() == 0)
		{
			// No midi inputs..
			midiBufferToUse = getFreeBuffer(true); // need to pick a buffer even if the processor doesn't use midi

			if (mapNode->getNode()->getProcessor()->acceptsMidi() || mapNode->getNode()->getProcessor()->producesMidi())
				renderingOps.add(new ClearMidiBufferOp(midiBufferToUse->index));
		} else if (midiSourceConnections.size() == 1)
		{
			const GraphRenderingOps::MapNodeConnection* mapNodeConnection = midiSourceConnections.getUnchecked(0);

			// One midi input..
			midiBufferToUse = getBufferContaining(mapNodeConnection->sourceMapNode->getNodeId(),
				NewAudioProcessorGraph::midiChannelIndex);

			if (midiBufferToUse >= 0)
			{
				if (isBufferNeededLater(midiBufferToUse, mapNode, NewAudioProcessorGraph::midiChannelIndex))
				{
					// can't mess up this channel because it's needed later by another node, so we
					// need to use a copy of it..
					const ChannelBufferInfo* newFreeBuffer = getFreeBuffer(true);
					renderingOps.add(new CopyMidiBufferOp(midiBufferToUse->index, newFreeBuffer->index));
					midiBufferToUse = newFreeBuffer;
				}
			} else
			{
				// probably a feedback loop, so just use an empty one..
				midiBufferToUse = getFreeBuffer(true); // need to pick a buffer even if the processor doesn't use midi
			}
		} else
		{
			// More than one midi input being mixed..
			int reusableInputIndex = -1;

			for (int i = 0; i < midiSourceConnections.size(); ++i)
			{

				const GraphRenderingOps::MapNodeConnection* midiConnection = midiSourceConnections.getUnchecked(i);

				const ChannelBufferInfo* sourceBuffer = getBufferContaining(midiConnection->sourceMapNode->getNodeId(),
					NewAudioProcessorGraph::midiChannelIndex);

				if (sourceBuffer != nullptr
					&& !isBufferNeededLater(sourceBuffer, mapNode, NewAudioProcessorGraph::midiChannelIndex))
				{
					// we've found one of our input buffers that can be re-used..
					reusableInputIndex = i;
					midiBufferToUse = sourceBuffer;
					break;
				}
			}

			if (reusableInputIndex < 0)
			{
				// can't re-use any of our input buffers, so get a new one and copy everything into it..
				midiBufferToUse = getFreeBuffer(true);
				jassert(midiBufferToUse >= 0);

				const GraphRenderingOps::MapNodeConnection* midiConnection = midiSourceConnections.getUnchecked(0);

				const ChannelBufferInfo* sourceBuffer = getBufferContaining(midiConnection->sourceMapNode->getNodeId(),
					NewAudioProcessorGraph::midiChannelIndex);
				if (sourceBuffer != nullptr)
					renderingOps.add(new CopyMidiBufferOp(sourceBuffer->index, midiBufferToUse->index));
				else
					renderingOps.add(new ClearMidiBufferOp(midiBufferToUse->index));

				reusableInputIndex = 0;
			}

			for (int j = 0; j < midiSourceConnections.size(); ++j)
			{

				const GraphRenderingOps::MapNodeConnection* midiConnection = midiSourceConnections.getUnchecked(j);

				if (j != reusableInputIndex)
				{
					const ChannelBufferInfo* sourceBuffer = getBufferContaining(midiConnection->sourceMapNode->getNodeId(),
						NewAudioProcessorGraph::midiChannelIndex);
					if (sourceBuffer != nullptr)
						renderingOps.add(new AddMidiBufferOp(sourceBuffer->index, midiBufferToUse->index));
				}
			}
		}

		if (mapNode->getNode()->getProcessor()->producesMidi())
			markBufferAsContaining(midiBufferToUse->index, mapNode,
				NewAudioProcessorGraph::midiChannelIndex);

		if (numOuts == 0)
			totalLatency = maxInputLatency;

		renderingOps.add(new ProcessBufferOp(mapNode->getNode(), audioChannelsToUse,
			totalChans, midiBufferToUse->index));
	}

	//==============================================================================
	const ChannelBufferInfo* getFreeBuffer(const bool forMidi)
	{
		if (forMidi)
		{
			for (int i = 1; i < midiChannelBuffers.size(); ++i)
				if (midiChannelBuffers.getUnchecked(i)->nodeId == freeNodeID)
					return midiChannelBuffers.getUnchecked(i);

			midiChannelBuffers.add(new ChannelBufferInfo(midiChannelBuffers.size(), (uint32)freeNodeID, 0, 0));
			return midiChannelBuffers.getUnchecked(midiChannelBuffers.size() - 1);
		} else
		{
			for (int i = 1; i < audioChannelBuffers.size(); ++i)
			{
				const ChannelBufferInfo* info = audioChannelBuffers.getUnchecked(i);
				if (info->nodeId == freeNodeID)
					return info;
			}

			audioChannelBuffers.add(new ChannelBufferInfo(audioChannelBuffers.size(), (uint32)freeNodeID, 0, 0));
			return audioChannelBuffers.getUnchecked(audioChannelBuffers.size() - 1);

		}
	}

	const ChannelBufferInfo* getReadOnlyEmptyBuffer() const noexcept
	{
		return audioChannelBuffers.getUnchecked(0);
	}

	const ChannelBufferInfo* getBufferContaining(const uint32 nodeId, const uint32 outputChannel) const noexcept
	{
		if (outputChannel == NewAudioProcessorGraph::midiChannelIndex)
		{
			for (int i = midiChannelBuffers.size(); --i >= 0;)
				if (midiChannelBuffers.getUnchecked(i)->nodeId == nodeId)
					return midiChannelBuffers.getUnchecked(i);
		} else
		{
			for (int i = audioChannelBuffers.size(); --i >= 0;)
			{
				const ChannelBufferInfo* info = audioChannelBuffers.getUnchecked(i);
				if (info->nodeId == nodeId && info->channelIndex == outputChannel)
					return info;
			}
		}

		return nullptr;
	}

	bool isBufferNeededLater(const ChannelBufferInfo* channelBuffer,
		const MapNode* currentMapNode,
		int inputChannelOfCurrentMapNodeToIgnore) const
	{
		//Needed by another node later on, easy.
		if (channelBuffer->freeAtRenderIndex > currentMapNode->getRenderIndex())
			return true;

		//Could be connected to multiple inputs on the current node, see if it is connected to a channel other than
		//the specified one to ignore
		if (isNodeBusy(channelBuffer->nodeId))
		{
			jassert(channelBuffer->mapNode != nullptr);

			const MapNode* srcMapNode = channelBuffer->mapNode;
			const uint32 outputChanIndex = channelBuffer->channelIndex;

			for (int i = 0; i < srcMapNode->getDestConnections().size(); ++i)
			{
				const MapNodeConnection* ec = srcMapNode->getDestConnections().getUnchecked(i);

				if (ec->destMapNode->getNodeId() == currentMapNode->getNodeId())
				{
					if (outputChanIndex == NewAudioProcessorGraph::midiChannelIndex)
					{
						if (inputChannelOfCurrentMapNodeToIgnore != NewAudioProcessorGraph::midiChannelIndex
							&& ec->sourceChannelIndex == NewAudioProcessorGraph::midiChannelIndex
							&& ec->destChannelIndex == NewAudioProcessorGraph::midiChannelIndex)
							return true;
					} else {
						if (ec->sourceChannelIndex == outputChanIndex
							&& ec->destChannelIndex != inputChannelOfCurrentMapNodeToIgnore)
							return true;
					}

				}

			}

		}

		return false;

	}

	void markAnyUnusedBuffersAsFree(const MapNode* currentMapNode)
	{

		for (int i = 0; i < audioChannelBuffers.size(); ++i)
		{
			const ChannelBufferInfo* info = audioChannelBuffers.getUnchecked(i);

			if (isNodeBusy(info->nodeId)
				&& currentMapNode->getRenderIndex() >= info->freeAtRenderIndex)
				audioChannelBuffers.set(i, new ChannelBufferInfo(i, (uint32)freeNodeID, info->channelIndex, 0));
		}

		for (int i = 0; i < midiChannelBuffers.size(); ++i)
		{
			const ChannelBufferInfo* info = midiChannelBuffers.getUnchecked(i);

			if (isNodeBusy(info->nodeId)
				&& currentMapNode->getRenderIndex() >= info->freeAtRenderIndex)
				midiChannelBuffers.set(i, new ChannelBufferInfo(i, (uint32)freeNodeID, 0, 0));
		}

	}

	void markBufferAsContaining(int bufferNum, const MapNode* mapNode, int outputIndex)
	{
		uint32 channelFreeAtIndex = 0;

		for (int i = 0; i < mapNode->getDestConnections().size(); ++i)
		{
			const MapNodeConnection* ec = mapNode->getDestConnections().getUnchecked(i);

			if (ec->sourceChannelIndex == outputIndex)
				channelFreeAtIndex = jmax(channelFreeAtIndex, ec->destMapNode->getRenderIndex());
		}

		if (outputIndex == NewAudioProcessorGraph::midiChannelIndex)
		{
			jassert(bufferNum > 0 && bufferNum < midiChannelBuffers.size());

			midiChannelBuffers.set(bufferNum, new ChannelBufferInfo(bufferNum, mapNode, NewAudioProcessorGraph::midiChannelIndex, channelFreeAtIndex));
		} else
		{
			jassert(bufferNum >= 0 && bufferNum < audioChannelBuffers.size());

			audioChannelBuffers.set(bufferNum, new ChannelBufferInfo(bufferNum, mapNode, outputIndex, channelFreeAtIndex));
		}
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RenderingOpSequenceCalculator);
};




//==============================================================================
struct ConnectionSorter
{
	static int compareElements(const NewAudioProcessorGraph::Connection* const first,
		const NewAudioProcessorGraph::Connection* const second) noexcept
	{
		if (first->sourceNodeId < second->sourceNodeId)                return -1;
		if (first->sourceNodeId > second->sourceNodeId)                return 1;
		if (first->destNodeId < second->destNodeId)                    return -1;
		if (first->destNodeId > second->destNodeId)                    return 1;
		if (first->sourceChannelIndex < second->sourceChannelIndex)    return -1;
		if (first->sourceChannelIndex > second->sourceChannelIndex)    return 1;
		if (first->destChannelIndex < second->destChannelIndex)        return -1;
		if (first->destChannelIndex > second->destChannelIndex)        return 1;

		return 0;
	}
};

}

//==============================================================================
NewAudioProcessorGraph::Connection::Connection(const uint32 sourceNodeId_, const int sourceChannelIndex_,
	const uint32 destNodeId_, const int destChannelIndex_) noexcept
	: sourceNodeId(sourceNodeId_), sourceChannelIndex(sourceChannelIndex_),
	destNodeId(destNodeId_), destChannelIndex(destChannelIndex_)
{
}

//==============================================================================
NewAudioProcessorGraph::Node::Node(const uint32 nodeId_, AudioProcessor* const processor_) noexcept
	: nodeId(nodeId_),
	processor(processor_),
	isPrepared(false)
{
	jassert(processor != nullptr);
}

void NewAudioProcessorGraph::Node::prepare(const double sampleRate, const int blockSize,
	NewAudioProcessorGraph* const graph)
{
	if (!isPrepared)
	{
		isPrepared = true;
		setParentGraph(graph);

		processor->setPlayConfigDetails(processor->getNumInputChannels(),
			processor->getNumOutputChannels(),
			sampleRate, blockSize);

		processor->prepareToPlay(sampleRate, blockSize);
	}
}

void NewAudioProcessorGraph::Node::unprepare()
{
	if (isPrepared)
	{
		isPrepared = false;
		processor->releaseResources();
	}
}

void NewAudioProcessorGraph::Node::setParentGraph(NewAudioProcessorGraph* const graph) const
{
	if (NewAudioProcessorGraph::AudioGraphIOProcessor* const ioProc
		= dynamic_cast <NewAudioProcessorGraph::AudioGraphIOProcessor*> (processor.get()))
		ioProc->setParentGraph(graph);
}

//==============================================================================
NewAudioProcessorGraph::NewAudioProcessorGraph()
	: lastNodeId(0),
	currentAudioInputBuffer(nullptr),
	currentMidiInputBuffer(nullptr)
{
}

NewAudioProcessorGraph::~NewAudioProcessorGraph()
{
	clearRenderingSequence();
	clear();
}

const String NewAudioProcessorGraph::getName() const
{
	return "Audio Graph";
}

//==============================================================================
void NewAudioProcessorGraph::clear()
{
	nodes.clear();
	connections.clear();
	triggerAsyncUpdate();
}

NewAudioProcessorGraph::Node* NewAudioProcessorGraph::getNodeForId(const uint32 nodeId) const
{
	for (int i = nodes.size(); --i >= 0;)
		if (nodes.getUnchecked(i)->nodeId == nodeId)
			return nodes.getUnchecked(i);

	return nullptr;
}

NewAudioProcessorGraph::Node* NewAudioProcessorGraph::addNode(AudioProcessor* const newProcessor, uint32 nodeId)
{
	if (newProcessor == nullptr || newProcessor == this)
	{
		jassertfalse;
		return nullptr;
	}

	for (int i = nodes.size(); --i >= 0;)
	{
		if (nodes.getUnchecked(i)->getProcessor() == newProcessor)
		{
			jassertfalse; // Cannot add the same object to the graph twice!
			return nullptr;
		}
	}

	if (nodeId == 0)
	{
		nodeId = ++lastNodeId;
	} else
	{
		// you can't add a node with an id that already exists in the graph..
		jassert(getNodeForId(nodeId) == nullptr);
		removeNode(nodeId);

		if (nodeId > lastNodeId)
			lastNodeId = nodeId;
	}

	newProcessor->setPlayHead(getPlayHead());

	Node* const n = new Node(nodeId, newProcessor);
	nodes.add(n);
	triggerAsyncUpdate();

	n->setParentGraph(this);
	return n;
}

bool NewAudioProcessorGraph::removeNode(const uint32 nodeId)
{
	disconnectNode(nodeId);

	for (int i = nodes.size(); --i >= 0;)
	{
		if (nodes.getUnchecked(i)->nodeId == nodeId)
		{
			nodes.getUnchecked(i)->setParentGraph(nullptr);
			nodes.remove(i);
			triggerAsyncUpdate();

			return true;
		}
	}

	return false;
}

//==============================================================================
const NewAudioProcessorGraph::Connection* NewAudioProcessorGraph::getConnectionBetween(const uint32 sourceNodeId,
	const int sourceChannelIndex,
	const uint32 destNodeId,
	const int destChannelIndex) const
{
	const Connection c(sourceNodeId, sourceChannelIndex, destNodeId, destChannelIndex);
	GraphRenderingOps::ConnectionSorter sorter;
	return connections[connections.indexOfSorted(sorter, &c)];
}

bool NewAudioProcessorGraph::isConnected(const uint32 possibleSourceNodeId,
	const uint32 possibleDestNodeId) const
{
	for (int i = connections.size(); --i >= 0;)
	{
		const Connection* const c = connections.getUnchecked(i);

		if (c->sourceNodeId == possibleSourceNodeId
			&& c->destNodeId == possibleDestNodeId)
		{
			return true;
		}
	}

	return false;
}

bool NewAudioProcessorGraph::canConnect(const uint32 sourceNodeId,
	const int sourceChannelIndex,
	const uint32 destNodeId,
	const int destChannelIndex) const
{
	if (sourceChannelIndex < 0
		|| destChannelIndex < 0
		|| sourceNodeId == destNodeId
		|| (destChannelIndex == midiChannelIndex) != (sourceChannelIndex == midiChannelIndex))
		return false;

	const Node* const source = getNodeForId(sourceNodeId);

	if (source == nullptr
		|| (sourceChannelIndex != midiChannelIndex && sourceChannelIndex >= source->processor->getNumOutputChannels())
		|| (sourceChannelIndex == midiChannelIndex && !source->processor->producesMidi()))
		return false;

	const Node* const dest = getNodeForId(destNodeId);

	if (dest == nullptr
		|| (destChannelIndex != midiChannelIndex && destChannelIndex >= dest->processor->getNumInputChannels())
		|| (destChannelIndex == midiChannelIndex && !dest->processor->acceptsMidi()))
		return false;

	return getConnectionBetween(sourceNodeId, sourceChannelIndex,
		destNodeId, destChannelIndex) == nullptr;
}

bool NewAudioProcessorGraph::addConnection(const uint32 sourceNodeId,
	const int sourceChannelIndex,
	const uint32 destNodeId,
	const int destChannelIndex)
{
	if (!canConnect(sourceNodeId, sourceChannelIndex, destNodeId, destChannelIndex))
		return false;

	GraphRenderingOps::ConnectionSorter sorter;
	connections.addSorted(sorter, new Connection(sourceNodeId, sourceChannelIndex,
		destNodeId, destChannelIndex));
	triggerAsyncUpdate();
	return true;
}

void NewAudioProcessorGraph::removeConnection(const int index)
{
	connections.remove(index);
	triggerAsyncUpdate();
}

bool NewAudioProcessorGraph::removeConnection(const uint32 sourceNodeId, const int sourceChannelIndex,
	const uint32 destNodeId, const int destChannelIndex)
{
	bool doneAnything = false;

	for (int i = connections.size(); --i >= 0;)
	{
		const Connection* const c = connections.getUnchecked(i);

		if (c->sourceNodeId == sourceNodeId
			&& c->destNodeId == destNodeId
			&& c->sourceChannelIndex == sourceChannelIndex
			&& c->destChannelIndex == destChannelIndex)
		{
			removeConnection(i);
			doneAnything = true;
		}
	}

	return doneAnything;
}

bool NewAudioProcessorGraph::disconnectNode(const uint32 nodeId)
{
	bool doneAnything = false;

	for (int i = connections.size(); --i >= 0;)
	{
		const Connection* const c = connections.getUnchecked(i);

		if (c->sourceNodeId == nodeId || c->destNodeId == nodeId)
		{
			removeConnection(i);
			doneAnything = true;
		}
	}

	return doneAnything;
}

bool NewAudioProcessorGraph::isConnectionLegal(const Connection* const c) const
{
	jassert(c != nullptr);

	const Node* const source = getNodeForId(c->sourceNodeId);
	const Node* const dest = getNodeForId(c->destNodeId);

	return source != nullptr
		&& dest != nullptr
		&& (c->sourceChannelIndex != midiChannelIndex ? isPositiveAndBelow(c->sourceChannelIndex, source->processor->getNumOutputChannels())
			: source->processor->producesMidi())
		&& (c->destChannelIndex != midiChannelIndex ? isPositiveAndBelow(c->destChannelIndex, dest->processor->getNumInputChannels())
			: dest->processor->acceptsMidi());
}

bool NewAudioProcessorGraph::removeIllegalConnections()
{
	bool doneAnything = false;

	for (int i = connections.size(); --i >= 0;)
	{
		if (!isConnectionLegal(connections.getUnchecked(i)))
		{
			removeConnection(i);
			doneAnything = true;
		}
	}

	return doneAnything;
}

//==============================================================================
static void deleteRenderOpArray(Array<void*>& ops)
{
	for (int i = ops.size(); --i >= 0;)
		delete static_cast<GraphRenderingOps::AudioGraphRenderingOp*> (ops.getUnchecked(i));
}

void NewAudioProcessorGraph::clearRenderingSequence()
{
	Array<void*> oldOps;

	{
		const ScopedLock sl(getCallbackLock());
		renderingOps.swapWith(oldOps);
	}

	deleteRenderOpArray(oldOps);
}

bool NewAudioProcessorGraph::isAnInputTo(const uint32 possibleInputId,
	const uint32 possibleDestinationId,
	const int recursionCheck) const
{
	if (recursionCheck > 0)
	{
		for (int i = connections.size(); --i >= 0;)
		{
			const NewAudioProcessorGraph::Connection* const c = connections.getUnchecked(i);

			if (c->destNodeId == possibleDestinationId
				&& (c->sourceNodeId == possibleInputId
					|| isAnInputTo(possibleInputId, c->sourceNodeId, recursionCheck - 1)))
				return true;
		}
	}

	return false;
}

void NewAudioProcessorGraph::buildRenderingSequence()
{
	Array<void*> newRenderingOps;
	int numRenderingBuffersNeeded = 2;
	int numMidiBuffersNeeded = 1;

	{
		MessageManagerLock mml;

		{
			for (int i = 0; i < nodes.size(); ++i)
			{
				Node* const node = nodes.getUnchecked(i);

				node->prepare(getSampleRate(), getBlockSize(), this);
			}
		}

		GraphRenderingOps::RenderingOpSequenceCalculator calculator(*this, nodes, connections, newRenderingOps);

		numRenderingBuffersNeeded = calculator.getNumBuffersNeeded();
		numMidiBuffersNeeded = calculator.getNumMidiBuffersNeeded();
	}

	{
		// swap over to the new rendering sequence..
		const ScopedLock sl(getCallbackLock());

		renderingBuffers.setSize(numRenderingBuffersNeeded, getBlockSize());
		renderingBuffers.clear();

		for (int i = midiBuffers.size(); --i >= 0;)
			midiBuffers.getUnchecked(i)->clear();

		while (midiBuffers.size() < numMidiBuffersNeeded)
			midiBuffers.add(new MidiBuffer());

		renderingOps.swapWith(newRenderingOps);
	}

	// delete the old ones..
	deleteRenderOpArray(newRenderingOps);
}

void NewAudioProcessorGraph::handleAsyncUpdate()
{
	buildRenderingSequence();
}

//==============================================================================
void NewAudioProcessorGraph::prepareToPlay(double /*sampleRate*/, int estimatedSamplesPerBlock)
{
	currentAudioInputBuffer = nullptr;
	currentAudioOutputBuffer.setSize(jmax(1, getNumOutputChannels()), estimatedSamplesPerBlock);
	currentMidiInputBuffer = nullptr;
	currentMidiOutputBuffer.clear();

	clearRenderingSequence();
	buildRenderingSequence();
}

void NewAudioProcessorGraph::releaseResources()
{
	for (int i = 0; i < nodes.size(); ++i)
		nodes.getUnchecked(i)->unprepare();

	renderingBuffers.setSize(1, 1);
	midiBuffers.clear();

	currentAudioInputBuffer = nullptr;
	currentAudioOutputBuffer.setSize(1, 1);
	currentMidiInputBuffer = nullptr;
	currentMidiOutputBuffer.clear();
}

void NewAudioProcessorGraph::reset()
{
	const ScopedLock sl(getCallbackLock());

	for (int i = 0; i < nodes.size(); ++i)
		nodes.getUnchecked(i)->getProcessor()->reset();
}

void NewAudioProcessorGraph::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	const int numSamples = buffer.getNumSamples();

	currentAudioInputBuffer = &buffer;
	currentAudioOutputBuffer.setSize(jmax(1, buffer.getNumChannels()), numSamples);
	currentAudioOutputBuffer.clear();
	currentMidiInputBuffer = &midiMessages;
	currentMidiOutputBuffer.clear();

	for (int i = 0; i < renderingOps.size(); ++i)
	{
		GraphRenderingOps::AudioGraphRenderingOp* const op
			= (GraphRenderingOps::AudioGraphRenderingOp*) renderingOps.getUnchecked(i);

		op->perform(renderingBuffers, midiBuffers, numSamples);
	}

	for (int i = 0; i < buffer.getNumChannels(); ++i)
		buffer.copyFrom(i, 0, currentAudioOutputBuffer, i, 0, numSamples);

	midiMessages.clear();
	midiMessages.addEvents(currentMidiOutputBuffer, 0, buffer.getNumSamples(), 0);
}

const String NewAudioProcessorGraph::getInputChannelName(int channelIndex) const
{
	return "Input " + String(channelIndex + 1);
}

const String NewAudioProcessorGraph::getOutputChannelName(int channelIndex) const
{
	return "Output " + String(channelIndex + 1);
}

bool NewAudioProcessorGraph::isInputChannelStereoPair(int /*index*/) const { return true; }
bool NewAudioProcessorGraph::isOutputChannelStereoPair(int /*index*/) const { return true; }
bool NewAudioProcessorGraph::silenceInProducesSilenceOut() const { return false; }
double NewAudioProcessorGraph::getTailLengthSeconds() const { return 0; }
bool NewAudioProcessorGraph::acceptsMidi() const { return true; }
bool NewAudioProcessorGraph::producesMidi() const { return true; }
void NewAudioProcessorGraph::getStateInformation(juce::MemoryBlock& /*destData*/) {}
void NewAudioProcessorGraph::setStateInformation(const void* /*data*/, int /*sizeInBytes*/) {}


//==============================================================================
NewAudioProcessorGraph::AudioGraphIOProcessor::AudioGraphIOProcessor(const IODeviceType type_)
	: type(type_),
	graph(nullptr)
{
}

NewAudioProcessorGraph::AudioGraphIOProcessor::~AudioGraphIOProcessor()
{
}

const String NewAudioProcessorGraph::AudioGraphIOProcessor::getName() const
{
	switch (type)
	{
		case audioOutputNode:   return "Audio Output";
		case audioInputNode:    return "Audio Input";
		case midiOutputNode:    return "Midi Output";
		case midiInputNode:     return "Midi Input";
		default:                break;
	}

	return String();
}

void NewAudioProcessorGraph::AudioGraphIOProcessor::fillInPluginDescription(PluginDescription& d) const
{
	d.name = getName();
	d.uid = d.name.hashCode();
	d.category = "I/O devices";
	d.pluginFormatName = "Internal";
	d.manufacturerName = "Raw Material Software";
	d.version = "1.0";
	d.isInstrument = false;

	d.numInputChannels = getNumInputChannels();
	if (type == audioOutputNode && graph != nullptr)
		d.numInputChannels = graph->getNumInputChannels();

	d.numOutputChannels = getNumOutputChannels();
	if (type == audioInputNode && graph != nullptr)
		d.numOutputChannels = graph->getNumOutputChannels();
}

void NewAudioProcessorGraph::AudioGraphIOProcessor::prepareToPlay(double, int)
{
	jassert(graph != nullptr);
}

void NewAudioProcessorGraph::AudioGraphIOProcessor::releaseResources()
{
}

void NewAudioProcessorGraph::AudioGraphIOProcessor::processBlock(AudioSampleBuffer& buffer,
	MidiBuffer& midiMessages)
{
	jassert(graph != nullptr);

	switch (type)
	{
		case audioOutputNode:
		{
			for (int i = jmin(graph->currentAudioOutputBuffer.getNumChannels(),
				buffer.getNumChannels()); --i >= 0;)
			{
				graph->currentAudioOutputBuffer.addFrom(i, 0, buffer, i, 0, buffer.getNumSamples());
			}

			break;
		}

		case audioInputNode:
		{
			for (int i = jmin(graph->currentAudioInputBuffer->getNumChannels(),
				buffer.getNumChannels()); --i >= 0;)
			{
				buffer.copyFrom(i, 0, *graph->currentAudioInputBuffer, i, 0, buffer.getNumSamples());
			}

			break;
		}

		case midiOutputNode:
			graph->currentMidiOutputBuffer.addEvents(midiMessages, 0, buffer.getNumSamples(), 0);
			break;

		case midiInputNode:
			midiMessages.addEvents(*graph->currentMidiInputBuffer, 0, buffer.getNumSamples(), 0);
			break;

		default:
			break;
	}
}

bool NewAudioProcessorGraph::AudioGraphIOProcessor::silenceInProducesSilenceOut() const
{
	return isOutput();
}

double NewAudioProcessorGraph::AudioGraphIOProcessor::getTailLengthSeconds() const
{
	return 0;
}

bool NewAudioProcessorGraph::AudioGraphIOProcessor::acceptsMidi() const
{
	return type == midiOutputNode;
}

bool NewAudioProcessorGraph::AudioGraphIOProcessor::producesMidi() const
{
	return type == midiInputNode;
}

const String NewAudioProcessorGraph::AudioGraphIOProcessor::getInputChannelName(int channelIndex) const
{
	switch (type)
	{
		case audioOutputNode:   return "Output " + String(channelIndex + 1);
		case midiOutputNode:    return "Midi Output";
		default:                break;
	}

	return String();
}

const String NewAudioProcessorGraph::AudioGraphIOProcessor::getOutputChannelName(int channelIndex) const
{
	switch (type)
	{
		case audioInputNode:    return "Input " + String(channelIndex + 1);
		case midiInputNode:     return "Midi Input";
		default:                break;
	}

	return String();
}

bool NewAudioProcessorGraph::AudioGraphIOProcessor::isInputChannelStereoPair(int /*index*/) const
{
	return type == audioInputNode || type == audioOutputNode;
}

bool NewAudioProcessorGraph::AudioGraphIOProcessor::isOutputChannelStereoPair(int index) const
{
	return isInputChannelStereoPair(index);
}

bool NewAudioProcessorGraph::AudioGraphIOProcessor::isInput() const { return type == audioInputNode || type == midiInputNode; }
bool NewAudioProcessorGraph::AudioGraphIOProcessor::isOutput() const { return type == audioOutputNode || type == midiOutputNode; }

bool NewAudioProcessorGraph::AudioGraphIOProcessor::hasEditor() const { return false; }
AudioProcessorEditor* NewAudioProcessorGraph::AudioGraphIOProcessor::createEditor() { return nullptr; }

int NewAudioProcessorGraph::AudioGraphIOProcessor::getNumParameters() { return 0; }
const String NewAudioProcessorGraph::AudioGraphIOProcessor::getParameterName(int) { return String(); }

float NewAudioProcessorGraph::AudioGraphIOProcessor::getParameter(int) { return 0.0f; }
const String NewAudioProcessorGraph::AudioGraphIOProcessor::getParameterText(int) { return String(); }
void NewAudioProcessorGraph::AudioGraphIOProcessor::setParameter(int, float) {}

int NewAudioProcessorGraph::AudioGraphIOProcessor::getNumPrograms() { return 0; }
int NewAudioProcessorGraph::AudioGraphIOProcessor::getCurrentProgram() { return 0; }
void NewAudioProcessorGraph::AudioGraphIOProcessor::setCurrentProgram(int) {}

const String NewAudioProcessorGraph::AudioGraphIOProcessor::getProgramName(int) { return String(); }
void NewAudioProcessorGraph::AudioGraphIOProcessor::changeProgramName(int, const String&) {}

void NewAudioProcessorGraph::AudioGraphIOProcessor::getStateInformation(juce::MemoryBlock&) {}
void NewAudioProcessorGraph::AudioGraphIOProcessor::setStateInformation(const void*, int) {}

void NewAudioProcessorGraph::AudioGraphIOProcessor::setParentGraph(NewAudioProcessorGraph* const newGraph)
{
	graph = newGraph;

	if (graph != nullptr)
	{
		setPlayConfigDetails(type == audioOutputNode ? graph->getNumOutputChannels() : 0,
			type == audioInputNode ? graph->getNumInputChannels() : 0,
			getSampleRate(),
			getBlockSize());

		updateHostDisplay();
	}
}