using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using NobleLauncher.Models;

namespace NobleLauncher.Services
{
    public class ProcessSupervisor
    {
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct STARTUPINFO
        {
            public int cb;
            public string lpReserved;
            public string lpDesktop;
            public string lpTitle;
            public int dwX;
            public int dwY;
            public int dwXSize;
            public int dwYSize;
            public int dwXCountChars;
            public int dwYCountChars;
            public int dwFillAttribute;
            public int dwFlags;
            public short wShowWindow;
            public short cbReserved2;
            public IntPtr lpReserved2;
            public IntPtr hStdInput;
            public IntPtr hStdOutput;
            public IntPtr hStdError;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct PROCESS_INFORMATION
        {
            public IntPtr hProcess;
            public IntPtr hThread;
            public int dwProcessId;
            public int dwThreadId;
        }

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern bool CreateProcessW(
            string? lpApplicationName,
            StringBuilder lpCommandLine,
            IntPtr lpProcessAttributes,
            IntPtr lpThreadAttributes,
            bool bInheritHandles,
            uint dwCreationFlags,
            IntPtr lpEnvironment,
            string? lpCurrentDirectory,
            ref STARTUPINFO lpStartupInfo,
            out PROCESS_INFORMATION lpProcessInformation);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern uint ResumeThread(IntPtr hThread);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr hObject);

        private const uint CREATE_SUSPENDED = 0x00000004;

        public event Action<string>? LogMessageReceived;
        public event Action<int>? ProcessExited;

        public bool IsRunning { get; private set; }
        public int RunningProcessId { get; private set; }

        public string FindExecutable(bool dedicated = false)
        {
            string exeName = dedicated ? "noble-dedicated.exe" : "noble-client.exe";

            var baseDir = AppDomain.CurrentDomain.BaseDirectory;
            var searchPaths = new[]
            {
                Path.Combine(baseDir, exeName),
                Path.Combine(baseDir, "bin", exeName),
                Path.Combine(baseDir, "..", "bin", exeName),
                Path.Combine(baseDir, "..", "..", "bin", exeName),
                Path.Combine(baseDir, "..", "..", "..", "bin", exeName),
                Path.Combine(baseDir, "build", "bin", exeName),
                Path.Combine(baseDir, "..", "build", "bin", exeName),
                Path.Combine(baseDir, "..", "..", "build", "bin", exeName),
                Path.Combine(baseDir, "..", "..", "..", "build", "bin", exeName),
                Path.Combine(baseDir, "build", "bin", "Release", exeName),
                Path.Combine(baseDir, "..", "build", "bin", "Release", exeName),
                Path.Combine(baseDir, "..", "..", "build", "bin", "Release", exeName),
                Path.Combine(Directory.GetCurrentDirectory(), "bin", exeName),
                Path.Combine(Directory.GetCurrentDirectory(), "build", "bin", exeName),
                Path.Combine(Directory.GetCurrentDirectory(), exeName)
            };

            foreach (var path in searchPaths)
            {
                var fullPath = Path.GetFullPath(path);
                if (File.Exists(fullPath))
                {
                    return fullPath;
                }
            }

            return Path.GetFullPath(Path.Combine(Directory.GetCurrentDirectory(), "bin", exeName));
        }

        public bool Launch(LauncherConfig config, GameInstallation installation, bool dedicated = false,
                           string? customMap = null, string? customMode = null, string? customMvar = null, string? customVariant = null)
        {
            var exePath = FindExecutable(dedicated);
            if (!File.Exists(exePath))
            {
                Log($"Error: Target executable '{exePath}' does not exist. Please compile noble-client.exe or noble-dedicated.exe first.");
                return false;
            }

            // Construct command line arguments
            var cmdBuilder = new StringBuilder();
            cmdBuilder.Append($"\"{exePath}\"");

            if (dedicated)
            {
                cmdBuilder.Append($" -dedicated -port {config.DedicatedPort}");
                if (!string.IsNullOrWhiteSpace(config.DedicatedRconPassword))
                {
                    cmdBuilder.Append($" -rcon \"{config.DedicatedRconPassword}\"");
                }
            }
            else
            {
                if (config.LaunchWindowed)
                {
                    cmdBuilder.Append($" -windowed -width {config.WindowWidth} -height {config.WindowHeight}");
                }
                if (config.DisableVsync)
                {
                    cmdBuilder.Append(" -novsync");
                }
                if (!config.EnableVfs)
                {
                    cmdBuilder.Append(" -novfs");
                }
                if (!config.EnableExpandedLobby)
                {
                    cmdBuilder.Append(" -vanillalobby");
                }
                if (!string.IsNullOrWhiteSpace(customMap))
                {
                    cmdBuilder.Append($" -map \"{customMap}\"");
                }
                if (!string.IsNullOrWhiteSpace(customMode))
                {
                    cmdBuilder.Append($" -mode \"{customMode}\"");
                }
                if (!string.IsNullOrWhiteSpace(customMvar))
                {
                    cmdBuilder.Append($" -mvar \"{customMvar}\"");
                }
                if (!string.IsNullOrWhiteSpace(customVariant))
                {
                    cmdBuilder.Append($" -variant \"{customVariant}\"");
                }
            }

            if (!string.IsNullOrWhiteSpace(installation.MccRootPath))
            {
                cmdBuilder.Append($" -mccpath \"{installation.MccRootPath}\"");
            }

            var workingDir = Path.GetDirectoryName(exePath) ?? Directory.GetCurrentDirectory();

            Log($"Launching: {cmdBuilder}");
            Log($"Working Directory: {workingDir}");

            if (config.StartSuspendedForInjector)
            {
                return LaunchSuspended(exePath, cmdBuilder.ToString(), workingDir);
            }
            else
            {
                return LaunchStandard(exePath, cmdBuilder.ToString(), workingDir);
            }
        }

        private bool LaunchSuspended(string exePath, string commandLine, string workingDir)
        {
            var si = new STARTUPINFO();
            si.cb = Marshal.SizeOf(si);

            var cmdBuilder = new StringBuilder(commandLine);

            bool success = CreateProcessW(
                null,
                cmdBuilder,
                IntPtr.Zero,
                IntPtr.Zero,
                false,
                CREATE_SUSPENDED,
                IntPtr.Zero,
                workingDir,
                ref si,
                out var pi);

            if (!success)
            {
                int error = Marshal.GetLastWin32Error();
                Log($"CreateProcessW failed with Win32 error: {error}");
                return false;
            }

            RunningProcessId = pi.dwProcessId;
            IsRunning = true;
            Log($"Process spawned in SUSPENDED state (PID: {pi.dwProcessId}, Thread: {pi.dwThreadId}).");
            Log("Injecting memory detours and hook payload before main thread resume...");

            // Resume the main thread
            uint prevCount = ResumeThread(pi.hThread);
            Log($"Thread resumed (Suspend count before resume: {prevCount}). Process is now running.");

            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);

            MonitorProcess(pi.dwProcessId);
            return true;
        }

        private bool LaunchStandard(string exePath, string commandLine, string workingDir)
        {
            try
            {
                var psi = new ProcessStartInfo
                {
                    FileName = exePath,
                    Arguments = commandLine.Substring(commandLine.IndexOf(' ') + 1),
                    WorkingDirectory = workingDir,
                    UseShellExecute = false
                };

                var proc = Process.Start(psi);
                if (proc != null)
                {
                    RunningProcessId = proc.Id;
                    IsRunning = true;
                    Log($"Process started directly (PID: {proc.Id}).");
                    MonitorProcess(proc.Id);
                    return true;
                }
            }
            catch (Exception ex)
            {
                Log($"Failed to launch process: {ex.Message}");
            }

            return false;
        }

        private void MonitorProcess(int pid)
        {
            System.Threading.Tasks.Task.Run(() =>
            {
                try
                {
                    var proc = Process.GetProcessById(pid);
                    proc.WaitForExit();
                    int exitCode = proc.ExitCode;
                    IsRunning = false;
                    Log($"Process {pid} exited with code {exitCode}.");
                    ProcessExited?.Invoke(exitCode);
                }
                catch
                {
                    IsRunning = false;
                }
            });
        }

        private void Log(string msg)
        {
            LogMessageReceived?.Invoke($"[{DateTime.Now:HH:mm:ss}] {msg}");
        }
    }
}
