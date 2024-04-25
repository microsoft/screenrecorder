namespace GAUSS
{
    public interface ICommandFactory
    {
        public string CommandName { get; }
        public string CommandHelp { get; }
        ICommand? Create(string[] args);
    }
}
