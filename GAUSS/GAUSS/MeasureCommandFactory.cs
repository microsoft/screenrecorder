using Microsoft.Windows.EventTracing;

namespace GAUSS
{
    public class MeasureCommandFactory : ICommandFactory
    {
        public string CommandName => "measure";

        public string CommandHelp => @"
  GAUSS -measure ...    Starts GAUSS measurment of a scenario duration
        Usage:  GAUSS -measure <duration> <framerate>
                        [-output <folder path>]
        Ex>     GAUSS -measure 5 2
        Ex>     GAUSS -measure 20 1 -output C:\gaussoutput

        <duration>      Duration for which to record the target scenario in seconds.
        <framerate>     Framerate at which to record the target scenario in FPS. 
        -outputpath	    Specifies the folder to which to save screenshots and trace
			            of the scenario measured.";

        public ICommand? Create(string[] args)
        {
            if (args.Length < 2)
            {
                return null;
            }

            if (!int.TryParse(args[0], out int durationParse))
            {
                return null;
            }

            Duration duration = Duration.FromSeconds(durationParse);

            if (!double.TryParse(args[1], out double framerate))
            {
                return null;
            }

            MeasureCommand command = new MeasureCommand(duration, framerate);

            for (int i = 2; i < args.Length; i++)
            {
                if (args[i].Equals("-outputpath"))
                {
                    command.OutputPath = args[++i];
                }
                else
                {
                    return null;
                }
            }

            return command;
        }
    }
}
