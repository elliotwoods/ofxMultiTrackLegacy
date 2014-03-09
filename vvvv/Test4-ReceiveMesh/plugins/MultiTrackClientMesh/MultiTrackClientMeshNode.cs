#region usings
using System;
using System.IO;
using System.ComponentModel.Composition;

using VVVV.PluginInterfaces.V1;
using VVVV.PluginInterfaces.V2;
using VVVV.Utils.VColor;
using VVVV.Utils.VMath;

using VVVV.Core.Logging;
#endregion usings

namespace VVVV.Nodes
{
	#region PluginInfo
	[PluginInfo(Name = "ClientMesh", Category = "MultiTrack", Help = "Construct meshes out of incoming streams", Tags = "")]
	#endregion PluginInfo
	public class MultiTrackClientMeshNode : IPluginEvaluate, IPartImportsSatisfiedNotification
	{
		#region fields & pins
		[Input("Input", IsSingle = true)]
		public ISpread<Stream> FInStream;
		
		[Input("Node Index")]
		public ISpread<int> FInNodeIndex;
		
		[Output("Transform")]
		public ISpread<Matrix4x4> FOutTransform;
		
		[Output("Vertices")]
		public ISpread<ISpread<Vector3D>> FOutVertices;
		
		[Output("Indices")]
		public ISpread<ISpread<int>> FOutIndices;
		
		//when dealing with byte streams (what we call Raw in the GUI) it's always
		//good to have a byte buffer around. we'll use it when copying the data.
		readonly byte[] FBuffer = new byte[1024];
		#endregion fields & pins

		//called when all inputs and outputs defined above are assigned from the host
		public void OnImportsSatisfied()
		{
		}

		//called when data for any output pin is requested
		public unsafe void Evaluate(int spreadMax)
		{
			if (FOutIndices.SliceCount != FInNodeIndex.SliceCount) {
				FOutIndices.SliceCount = FInNodeIndex.SliceCount;
				FOutVertices.SliceCount = FInNodeIndex.SliceCount;
				FOutTransform.SliceCount = FInNodeIndex.SliceCount;
			}
			
			foreach(var stream in FInStream) {
				if (stream == null) {
					return;
				}
				if (stream.Length == 0) {
					return;
				}
				var buffer = new byte[stream.Length];
				stream.Read(buffer, 0, (int) stream.Length);
				var remainingBytes = stream.Length;
				
				fixed(byte * fixedData = & buffer[0]) {
					var data = fixedData;
					
					//numIndices
					if (remainingBytes < sizeof(int)) {
						return;
					}
					int nodeIndex = * (int *) data;
					data += sizeof(int);
					remainingBytes -= sizeof(int);
					
					int sliceIndex = -1;
					for(int i=0; i<FInNodeIndex.SliceCount; i++) {
						if (FInNodeIndex[i] == nodeIndex) {
							sliceIndex = i;
							break;
						}
					}
					if (sliceIndex == -1) {
						continue;
					}
					
					//transform
					if (remainingBytes < sizeof(float) * 16) {
						return;
					}
					Matrix4x4 matrix = new Matrix4x4();
					for(int i=0; i<16; i++) {
						matrix[i] = (double) * (float *) data;
						data += sizeof(float);
						remainingBytes -= sizeof(float);
					}
					FOutTransform[sliceIndex] = matrix;
					
					//numIndices
					if (remainingBytes < sizeof(int)) {
						return;
					}
					int numIndices;
					numIndices = * (int *) data;
					data += sizeof(int);
					remainingBytes -= sizeof(int);
					FOutIndices[sliceIndex].SliceCount = numIndices;
					
					//indices
					if (remainingBytes < sizeof(UInt32) * numIndices) {
						return;
					}
					for(int i=0; i<numIndices; i++) {
						FOutIndices[sliceIndex][i] = (int) ((UInt32*)data)[i];
					}
					data += sizeof(UInt32) * numIndices;
					remainingBytes -= sizeof(UInt32) * numIndices;
					
					//numVertices
					if (remainingBytes < sizeof(int)) {
						return;
					}
					int numVertices;
					numVertices = * (int *) data;
					data += sizeof(int);
					remainingBytes -= sizeof(int);
					FOutVertices[sliceIndex].SliceCount = numVertices;
					
					//vertices
					if (remainingBytes < sizeof(float) * numVertices * 3) {
						return;
					}
					for(int i=0; i<numVertices; i++) {
						float * vertexPointer = (float*) data;
						FOutVertices[sliceIndex][i] = new Vector3D(
							(double) ((float*) data)[0],
							(double) ((float*) data)[1],
							(double) ((float*) data)[2]);
							
						data += sizeof(float) * 3;
					}
					remainingBytes -= sizeof(UInt32) * numIndices;
				}
			}
		}
	}
}
