using Microsoft.Windows.EventTracing;
using System.ComponentModel.Design;

namespace GAUSS
{
    public class HelpCommandFactory : ICommandFactory
    {
        public string CommandName => "help";

        public string CommandHelp => @"
        Usage: GAUSS options ...

        -help measure       - for GAUSS measure command";

        private List<ICommandFactory> commandFactories;

        public HelpCommandFactory(List<ICommandFactory> commandFactories)
        {
            this.commandFactories = commandFactories;
        }

        public ICommand? Create(string[] args)
        {
            if (args.Length != 1)
            {
                return null;
            }

            List<ICommandFactory> matchingFactories = commandFactories.Where(f => f.CommandName.Equals(args[0])).ToList();

            if (matchingFactories.Count != 1)
            {
                return null;
            }

            return new HelpCommand(matchingFactories[0]);
        }
    }
}
