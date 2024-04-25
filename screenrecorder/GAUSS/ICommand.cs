using Microsoft.Windows.EventTracing;
using Microsoft.Windows.EventTracing.Cpu;
using Microsoft.Windows.EventTracing.Events;
using Microsoft.Windows.EventTracing.Processes;
using System.ComponentModel;
using System.Diagnostics;

namespace GAUSS
{
    public interface ICommand
    {
        public bool CanPerform();
        public void Perform();
    }
}
