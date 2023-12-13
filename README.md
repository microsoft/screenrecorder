# GAUSS

## Table of Contents
- [Usage](#usage)
- [Capturing ETW Events](#capturing-etw-events)
- [Example Capture](#example-capture)

## Usage
The tool allows you to start and stop recording from the command line. When a recording is started, the framerate, monitor, and buffer size can be specified. When a recording is stopped, a folder must be provided in which to store the screenshots.

    screenrecorder.exe -start ...        Starts screen recording.
        Usage:  screenrecorder.exe -start [-framerate <framerate>] [-monitor <monitor index>] [-framebuffer -mb <# of frames>] [-monitor <monitor # to record>]
        Ex>     screenrecorder.exe -start -framerate 10
        Ex>     screenrecorder.exe -start -framerate 1 -monitor 0 -framebuffer -mb 100

        -framerate      Specifies the rate at which screenshots will be taken, in frames per second.
        -monitor        Specifies the monitor to record, as an index. The highest index will record all monitors.
        -framebuffer    Specifies the size of the circular memory buffer in which to store screenshots, in number of screenshots. Adding the -mb flag specifies the size of the buffer in megabytes.

    screenrecorder.exe -stop ...         Stops screen recording saves all screenshots in buffer to a folder.
        Usage:  screenrecorder.exe -stop <recording folder>
        Ex>     screenrecorder.exe -stop "D:\screenrecorder"

    screenrecorder.exe -cancel ...       Cancels the screen recording.

    screenrecorder.exe -help ...         Prints usage information.

## Capturing ETW Events
An ETW event is emitted by the tool every time it receives a frame from DirectX. The event for each frame contains the filename to be used if the frame is saved to disk, allowing for direct correlation between each event and screenshot. The tool does not receive frames from DirectX unless there has been a change in the screen, so desired framerates may not be exact.

To capture the ETW events, you must use a ETW tracing tool like WPR and watch for events from the following provider guid: fe8fc3d0-1e6a-42f2-be28-9f8a0fcf7b04.

## Example Capture

Create a WPR profile with the following provider and collectors defined:

    ...
    <EventProvider Id="screenrecorder" Name="fe8fc3d0-1e6a-42f2-be28-9f8a0fcf7b04">
	  </EventProvider>
    ...
    <EventCollectorId Value="Standard_EventCollector_Misc">
      <EventProviders>
        <EventProviderId Value="screenrecorder"/>
      </EventProviders>
    </EventCollectorId>
    ...

Take an ETW trace using WPR while you use the tool to capture some scenario.

    wpr -start profile.wprp
    ScreenRecorder.exe -start

    ... excercise some scenario of interest ...

    ScreenRecorder.exe -stop "D:\"
    wpr -stop trace.etl

Now opening the trace in WPA, we can see the events produced by the tool in the Generic Events table under the ScreenRecorder provider. Each event contains the filename for the screenshot.
