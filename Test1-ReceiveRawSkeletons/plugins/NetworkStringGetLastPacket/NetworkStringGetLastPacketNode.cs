#region usings
using System;
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
	[PluginInfo(Name = "GetLastPacket", Category = "String", Version = "Network", Help = "Basic template with one string in/out", Tags = "")]
	#endregion PluginInfo
	public class NetworkStringGetLastPacketNode : IPluginEvaluate
	{
		#region fields & pins
		[Input("Input", DefaultString = "hello c#")]
		public ISpread<string> FInput;

		[Input("Token")]
		public ISpread<string> FToken;
		
		[Output("Output")]
		public ISpread<string> FOutput;
		
		[Import()]
		public ILogger FLogger;
		#endregion fields & pins

		//called when data for any output pin is requested
		public void Evaluate(int SpreadMax)
		{
			FOutput.SliceCount = SpreadMax;
			
			for (int i = 0; i < SpreadMax; i++)
			{
				//find last one
				var lastInstanceOfToken = FInput[i].LastIndexOf(FToken[i]);
				if (lastInstanceOfToken != -1)
				{
					//find second to last one
					var secondToLastInstanceOfToken = FInput[i].Substring(0, lastInstanceOfToken).LastIndexOf(FToken[i]);
					if (secondToLastInstanceOfToken == -1) {
						FOutput[i] = FInput[i].Substring(0, lastInstanceOfToken);
					} else {
						FOutput[i] = FInput[i].Substring(secondToLastInstanceOfToken + FToken[i].Length, lastInstanceOfToken - secondToLastInstanceOfToken - FToken[i].Length);
					}
				} else {
					FOutput[i] = FInput[i];
				}
			}

			//FLogger.Log(LogType.Debug, "Logging to Renderer (TTY)");
		}
	}
}
