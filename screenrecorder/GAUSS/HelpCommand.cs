using Microsoft.Windows.EventTracing;

namespace GAUSS
{
    public class HelpCommand : ICommand
    {
        private ICommandFactory commandFactory;

        public HelpCommand(ICommandFactory commandFactory)
        {
            this.commandFactory = commandFactory;
        }

        public bool CanPerform()
        {
            return true;
        }

        public void Perform()
        {
            Console.WriteLine(commandFactory.CommandHelp);
        }
    }
}
