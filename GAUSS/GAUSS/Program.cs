namespace GAUSS
{
    public class Program
    {
        public static void Main(string[] args)
        {
            GAUSS gauss = new GAUSS();
            gauss.ProcessCommandLine(args);
        }
    }
}