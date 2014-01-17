#region usings
using System;
using System.ComponentModel.Composition;
using System.Net.Sockets;
using System.Runtime.Serialization.Json;
using System.Runtime.Serialization;

using VVVV.PluginInterfaces.V1;
using VVVV.PluginInterfaces.V2;
using VVVV.Utils.VColor;
using VVVV.Utils.VMath;

using VVVV.Core.Logging;
#endregion usings

namespace VVVV.Nodes
{
	#region PluginInfo
	[PluginInfo(Name = "Receive", Category = "MultiTrack", Help = "Basic template with one string in/out", Tags = "")]
	#endregion PluginInfo
	public class MultiTrackReceiveNode : IPluginEvaluate, IDisposable
	{
		#region fields & pins
		[Input("Client Address", DefaultString="localhost", IsSingle=true)]
		public IDiffSpread<string> FInAddress;
		
		[Input("Client Index", IsSingle=true)]
		public IDiffSpread<int> FInIndex;

		[Output("Output")]
		public ISpread<string> FOutput;
		
		[Output("Connected")]
		public ISpread<bool> FOutConnected;
		
		[Import()]
		public ILogger FLogger;
		
		TcpClient FClient = new TcpClient();
		byte[] FBuffer = new byte[100 * 1024];
		#endregion fields & pins

		static byte[] GetBytes(string str)
		{
			byte[] bytes = new byte[str.Length * sizeof(char)];
			System.Buffer.BlockCopy(str.ToCharArray(), 0, bytes, 0, bytes.Length);
			return bytes;
		}
			
		public void Evaluate(int SpreadMax)
		{
			FOutput.SliceCount = SpreadMax;
			
			if (FInAddress.IsChanged || FInIndex.IsChanged)
			{
				Connect();
			}
			var stream = FClient.GetStream();
			var receivedlength = stream.Read(FBuffer, 0, FBuffer.Length);
			var rawText = System.Text.Encoding.ASCII.GetString(FBuffer, 0, receivedlength);
			
			var tokenArray = new string[1];
			tokenArray[0] = "[/TCP]";
			var splitText = rawText.Split(tokenArray, StringSplitOptions.RemoveEmptyEntries);
			
			if (splitText.Length > 1) {
				var jsonText = splitText[splitText.Length-2];
				FOutput[0] = jsonText;
			}
			FOutConnected[0] = FClient.Connected;
		}
		
		void Connect()
		{
			Disconnect();
			
			FClient.Connect(FInAddress[0], FInIndex[0] + 2500);
		}
		
		void Disconnect()
		{
			if (FClient != null)
			{
				if (FClient.Connected)
				{
					FClient.Close();
				}
			}	
		}
		
		public void Dispose()
		{
			Disconnect();
		}
	}
}
