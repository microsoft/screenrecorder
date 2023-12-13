using Microsoft.Windows.EventTracing;
using System.Diagnostics;
using Microsoft.Windows.EventTracing.Processes;
using Microsoft.Windows.EventTracing.Cpu;
using Microsoft.Windows.EventTracing.Events;
using System.ComponentModel;

namespace GAUSS
{
    public class Program
    {
        public static readonly string WprExecutable = "wpr.exe";
        public static readonly string ScreenRecorderExecutable = "ScreenRecorder.exe";
        public static readonly string TraceProfile = "profile.wprp";
        public static readonly string LaunchDetectorScriptExecutable = "LaunchDetectorScript.exe";

        public static readonly string DefaultDataPath = Path.GetTempPath();
        public static readonly int DefaultFramerate = 2;
        public static readonly Duration DefaultRecordingDuration = Duration.FromMilliseconds(5000);

        public enum CommandLineOptions
        {
            DataPath,
            RecordingFramerate,
            RecordingDuration
        }

        public static readonly IReadOnlyDictionary<string, CommandLineOptions> OptionsByString = new Dictionary<string, CommandLineOptions>()
        {
            { "--data-path", CommandLineOptions.DataPath },
            { "--recording-framerate", CommandLineOptions.RecordingFramerate },
            { "--recording-duration", CommandLineOptions.RecordingDuration }
        };

        public static void Main(string[] args)
        {
            if (args.Length == 0 || args[0] != "--start")
            {
                Console.Write("Usage: MeasureInteraction.exe [--data-path <data output path>] [--recording-framerate <recording framerate>] [--recording-duration <recording duration>]\n"
                    + "\n"
                    + "--data-path\tSpecifies the location to store the interaction recorded. The default is to not store this.\n"
                    + "--recording-framerate\tSpecifies the framerate at which to record the interaction. The default is 2 frames per second.\n"
                    + "--recording-duration\tSpecifies the duration for which to record the interaction in milliseconds. The default is 5000 milliseconds.\n");

                return;
            }

            if (!TryGetArgs(args, out string dataPath, out bool keepData, out int framerate, out Duration recordingDuration))
            {
                Console.WriteLine("Failed to parse args.");
                Console.Write("Usage: MeasureInteraction.exe [--data-path <data output path>] [--recording-framerate <recording framerate>] [--recording-duration <recording duration>]\n"
                    + "\n"
                    + "--data-path\tSpecifies the location to store the interaction recorded. The default is to not store this.\n"
                    + "--recording-framerate\tSpecifies the framerate at which to record the interaction. The default is 2 frames per second.\n"
                    + "--recording-duration\tSpecifies the duration for which to record the interaction in milliseconds. The default is 5000 milliseconds.\n");

                return;
            }

            if (!Directory.Exists(dataPath))
            {
                Console.WriteLine($"Data path {dataPath} does not exist.");

                return;
            }

            string tracePath = Path.Combine(dataPath, "trace.etl");
            string screenshotsPath = Path.Combine(dataPath, "screenshots");

            if (File.Exists(tracePath))
            {
                Console.WriteLine($"Trace path {tracePath} already exists. Please delete or remove.");

                return;
            }

            if (Directory.Exists(screenshotsPath))
            {
                Console.WriteLine($"Screenshot folder {screenshotsPath} already exists. Please delete or remove.");

                return;
            }

            if (RunExecutable(WprExecutable, $"-start {TraceProfile}") != 0)
            {
                Console.WriteLine($"Failed to start WPR. Please check to make sure we are running as admin.");

                return;
            }

            Directory.CreateDirectory(screenshotsPath);
            int framebuffer = (int)(framerate * recordingDuration.TotalSeconds);

            if (RunExecutable(ScreenRecorderExecutable, $"-start -framerate {framerate} -framebuffer {framebuffer}") != 0)
            {
                Console.WriteLine($"Failed to start screen recorder. Does the executable exist?");

                return;
            }

            Console.WriteLine("Started recording...");

            Thread.Sleep((int)recordingDuration.TotalMilliseconds);

            Console.WriteLine("Stopping recording...");

            RunExecutable(ScreenRecorderExecutable, $"-stop {screenshotsPath}");
            RunExecutable(WprExecutable, $"-stop {tracePath}", out string wprstopout);

            Console.WriteLine("Processing data...");

            ITraceProcessor trace = TraceProcessor.Create(tracePath);

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
                    .Where(e => e.ProviderName == "Microsoft.Windows.Win32kBase.Input" && e.TaskName == "MousePacketLatency" && e.Fields[2].AsUInt32 == 1)
                    .OrderBy(e => e.Timestamp)
                    .Last().Timestamp.RelativeTimestamp;
            }
            catch (InvalidOperationException)
            {
                lastClickTimestamp = Timestamp.Zero;
            }

            if (RunExecutable(LaunchDetectorScriptExecutable, screenshotsPath, out string settledScreenshotName) != 0)
            {
                Console.WriteLine($"Failed to start screen recorder. Does the executable exist?");

                return;
            }

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

            if (!keepData)
            {
                File.Delete(tracePath);
                Directory.Delete(screenshotsPath, recursive: true);
            }
        }

        public static bool TryGetArgs(string[] args, out string dataPath, out bool keepData, out int framerate, out Duration recordingDuration)
        {
            try
            {
                GetArgs(args, out dataPath, out keepData, out framerate, out recordingDuration);

                return true;
            }
            catch (Exception)
            {
                SetArgsDefault(out dataPath, out keepData, out framerate, out recordingDuration);

                return false;
            }
        }

        public static void GetArgs(string[] args, out string dataPath, out bool keepData, out int framerate, out Duration recordingDuration)
        {
            SetArgsDefault(out dataPath, out keepData, out framerate, out recordingDuration);

            int i = 1;

            while (i < args.Length)
            {
                CommandLineOptions option = OptionsByString[args[i++]];

                switch (option)
                {
                    case CommandLineOptions.DataPath:
                        dataPath = args[i++];
                        keepData = true;

                        break;
                    case CommandLineOptions.RecordingFramerate:
                        framerate = int.Parse(args[i++]);

                        break;
                    case CommandLineOptions.RecordingDuration:
                        recordingDuration = Duration.FromMilliseconds(int.Parse(args[i++]));

                        break;
                    default:
                        throw new ArgumentException();
                }
            }
        }

        public static void SetArgsDefault(out string dataPath, out bool keepData, out int framerate, out Duration recordingDuration)
        {
            dataPath = DefaultDataPath;
            keepData = false;
            framerate = DefaultFramerate;
            recordingDuration = DefaultRecordingDuration;
        }

        public static int RunExecutable(string executablePath, string args)
        {
            ProcessStartInfo processStartInfo = new ProcessStartInfo
            {
                FileName = executablePath,
                Arguments = args
            };

            using Process process = new Process();
            process.StartInfo = processStartInfo;
            process.Start();
            process.WaitForExit();

            return process.ExitCode;
        }

        public static int RunExecutable(string executablePath, string args, out string stdout)
        {
            ProcessStartInfo processStartInfo = new ProcessStartInfo
            {
                RedirectStandardOutput = true,
                FileName = executablePath,
                Arguments = args
            };

            using Process process = new Process();
            process.StartInfo = processStartInfo;

            try
            {
                process.Start();
            }
            catch (Win32Exception)
            {
                stdout = string.Empty;

                return -1;
            }
            
            stdout = process.StandardOutput.ReadToEnd();
            process.WaitForExit();

            return process.ExitCode;
        }
    }
}