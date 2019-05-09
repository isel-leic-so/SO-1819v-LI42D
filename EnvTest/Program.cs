using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EnvTest {
    class Program {
        static void Main(string[] args) {
            foreach (System.Collections.DictionaryEntry env in Environment.GetEnvironmentVariables()) {
                string name = (string)env.Key;
                string value = (string)env.Value;
                Console.WriteLine("{0}={1}", name, value);
            }
        }
    }
}
