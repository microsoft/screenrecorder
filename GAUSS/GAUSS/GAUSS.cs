namespace GAUSS
{
    public class GAUSS
    {
        private ICommandFactory helpCommandFactory;
        private List<ICommandFactory> commandFactories;
        
        public GAUSS() 
        {
            commandFactories = new List<ICommandFactory>
            {
                new MeasureCommandFactory(),
            };

            helpCommandFactory = new HelpCommandFactory(commandFactories);
            commandFactories.Add(helpCommandFactory);
        }

        public void ProcessCommandLine(string[] commandLine)
        {
            if (commandLine.Length == 0)
            {
                Console.WriteLine(helpCommandFactory.CommandHelp);

                return;
            }

            string commandToken = commandLine[0];

            foreach(ICommandFactory commandFactory in commandFactories)
            {
                if (commandToken.Equals($"-{commandFactory.CommandName}"))
                {
                    ICommand? command = commandFactory.Create(commandLine.Skip(1).ToArray());

                    if (command == null) 
                    {
                        Console.WriteLine(commandFactory.CommandHelp);
                    }
                    else if (command.CanPerform())
                    {
                        command.Perform();
                    }

                    return;
                }
            }

            Console.WriteLine(helpCommandFactory.CommandHelp);
        }
    }
}
