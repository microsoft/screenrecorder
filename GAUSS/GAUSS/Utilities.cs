using System.ComponentModel;
using System.Diagnostics;
using System.Security.Principal;

namespace GAUSS
{
    public class Utilities
    {
        public static bool ExistsOnPath(string fileName)
        {
            return GetFullPath(fileName) != null;
        }

        public static string? GetFullPath(string fileName)
        {
            if (File.Exists(fileName))
            {
                return Path.GetFullPath(fileName);
            }     

            var values = Environment.GetEnvironmentVariable("PATH");

            if (values == null)
            {
                return null;
            }

            foreach (var path in values.Split(Path.PathSeparator))
            {
                var fullPath = Path.Combine(path, fileName);
                if (File.Exists(fullPath))
                {  
                    return fullPath; 
                }
            }

            return null;
        }

        public static bool IsWindowsAdmin()
        {
            if (System.Runtime.InteropServices.RuntimeInformation.IsOSPlatform(System.Runtime.InteropServices.OSPlatform.Windows))
            {
                WindowsIdentity identity = WindowsIdentity.GetCurrent();
                WindowsPrincipal principal = new WindowsPrincipal(identity);
                return principal.IsInRole(WindowsBuiltInRole.Administrator);
            }
            else
            {
                // For now, assume we are admin on other platforms
                return true;
            }
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
