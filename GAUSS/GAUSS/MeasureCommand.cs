using Microsoft.Windows.EventTracing;
using Microsoft.Windows.EventTracing.Cpu;
using Microsoft.Windows.EventTracing.Events;
using Microsoft.Windows.EventTracing.Processes;
using System.Text.RegularExpressions;

namespace GAUSS
{
    public class MeasureCommand : ICommand
    {
        public static readonly string WprExecutable = "wpr.exe";
        public static readonly string ScreenRecorderExecutable = "ScreenRecorder.exe";
        public static readonly string TraceProfile = "profile.wprp";
        public static readonly string LaunchDetectorScriptExecutable = "LaunchDetectorScript.exe";
        public static readonly string DefaultOutputPath = Path.GetTempPath();

        public Duration Duration { get; set; }

        public double Framerate { get; set; }

        public string OutputPath { get; set; }

        public string TracePath => Path.Combine(OutputPath, "trace.etl");

        public string ScreenshotsPath => Path.Combine(OutputPath, "screenshots");

        public MeasureCommand(Duration duration, double framerate) 
        { 
            Duration = duration;
            Framerate = framerate;
            OutputPath = DefaultOutputPath;
        }

        public bool CanPerform()
        {
            if (!Directory.Exists(OutputPath))
            {
                Console.WriteLine($"Data path {OutputPath} does not exist.");

                return false;
            }

            if (File.Exists(TracePath))
            {
                Console.WriteLine($"Trace path {TracePath} already exists. Please delete or remove.");

                return false;
            }

            if (Directory.Exists(ScreenshotsPath))
            {
                Console.WriteLine($"Screenshot folder {ScreenshotsPath} already exists. Please delete or remove.");

                return false;
            }

            if (!File.Exists(TraceProfile))
            {
                Console.WriteLine($"Failed to find trace profile {TraceProfile}.");

                return false;
            }

            if (!Utilities.ExistsOnPath(WprExecutable))
            {
                Console.WriteLine($"Failed to find {WprExecutable} on path.");

                return false;
            }

            if (!Utilities.ExistsOnPath(ScreenRecorderExecutable))
            {
                Console.WriteLine($"Failed to find {ScreenRecorderExecutable} on path.");

                return false;
            }

            if (!Utilities.ExistsOnPath(LaunchDetectorScriptExecutable))
            {
                Console.WriteLine($"Failed to find {LaunchDetectorScriptExecutable} on path.");

                return false;
            }

            if (!Utilities.IsWindowsAdmin())
            {
                Console.WriteLine($"Please run as admin to measure a scenario.");

                return false;
            }

            return true;
        }

        public void Perform()
        {
            if (!CanPerform())
            {
                return;
            }

            if (Utilities.RunExecutable(WprExecutable, $"-start {TraceProfile}") != 0)
            {
                Console.WriteLine($"WPR failed.");

                return;
            }

            Directory.CreateDirectory(ScreenshotsPath);
            int framebuffer = (int)((decimal)Framerate * Duration.TotalSeconds);

            if (Utilities.RunExecutable(ScreenRecorderExecutable, $"-start -framerate {Framerate} -framebuffer {framebuffer}") != 0)
            {
                Console.WriteLine($"ScreenRecorder failed.");

                return;
            }

            Console.WriteLine("Started recording...");

            Thread.Sleep((int)Duration.TotalMilliseconds);

            Console.WriteLine("Stopping recording...");

            if (Utilities.RunExecutable(ScreenRecorderExecutable, $"-stop {ScreenshotsPath}") != 0)
            {
                Console.WriteLine($"ScreenRecorder failed.");

                return;
            }

            if (Utilities.RunExecutable(WprExecutable, $"-stop {TracePath}", out string wprstopout) != 0)
            {
                Console.WriteLine($"WPR failed.");

                return;
            }

            Console.WriteLine("Processing data...");

            ITraceProcessor trace = TraceProcessor.Create(TracePath);

            IPendingResult<IProcessDataSource> pendingProcessDataSource = trace.UseProcesses();
            IPendingResult<ICpuSampleDataSource> pendingCpuSampleDataSource = trace.UseCpuSamplingData();
            IPendingResult<IGenericEventDataSource> pendingGenericEventDataSource = trace.UseGenericEvents();

            trace.Process();

            IProcessDataSource processDataSource = pendingProcessDataSource.Result;
            ICpuSampleDataSource cpuSampleDataSource = pendingCpuSampleDataSource.Result;
            IGenericEventDataSource genericEventDataSource = pendingGenericEventDataSource.Result;

            Timestamp lastClickTimestamp;

            try
            {
                lastClickTimestamp = genericEventDataSource.Events
                    .Where(e => e.ProviderName == "Microsoft.Windows.Win32kBase.Input" && e.TaskName == "MousePacketLatency" && e.Fields[1].AsUInt32 == 1)
                    .OrderBy(e => e.Timestamp)
                    .Last().Timestamp.RelativeTimestamp;
            }
            catch (InvalidOperationException)
            {
                lastClickTimestamp = Timestamp.Zero;
            }

            if (Utilities.RunExecutable(LaunchDetectorScriptExecutable, ScreenshotsPath, out string launchDetectorScriptOutput) != 0)
            {
                Console.WriteLine($"LaunchDetectorScript failed.");

                return;
            }

            string pattern = @"[a-zA-Z0-9_-]*\.jpg";

            Match match = Regex.Match(launchDetectorScriptOutput, pattern);
            string settledScreenshotName = match.Value;

            Timestamp settledTimestamp;

            try
            {
                settledTimestamp = genericEventDataSource.Events
                    .Where(e => e.ProviderName == "ScreenRecorder" && e.ActivityName == "ReceivedFrame")
                    .Where(e => e.Fields[0].AsString == settledScreenshotName).Single().Timestamp.RelativeTimestamp;
            }
            catch (InvalidOperationException)
            {
                settledTimestamp = Timestamp.Zero;
            }

            Console.WriteLine($"Settled screenshot name: {settledScreenshotName}");

            if (lastClickTimestamp == Timestamp.Zero || settledTimestamp == Timestamp.Zero)
            {
                Console.WriteLine(lastClickTimestamp == Timestamp.Zero ? $"No clicks found in trace." : $"lastClickTimestamp: {lastClickTimestamp.TotalSeconds} seconds.");
                Console.WriteLine(settledTimestamp == Timestamp.Zero ? $"No settled screenshot found in trace." : $"settledTimestamp: {settledTimestamp.TotalSeconds} seconds.");
            }
            else
            {
                Duration interactionDuration = settledTimestamp - lastClickTimestamp;
                Console.WriteLine($"Measured interaction duration: {interactionDuration.TotalSeconds} seconds");
            }

            if (OutputPath.Equals(DefaultOutputPath))
            {
                File.Delete(TracePath);
                Directory.Delete(ScreenshotsPath, recursive: true);
            }
        }
    }
}
