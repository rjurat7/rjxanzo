#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
int main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    rps33::MainForm^ form = gcnew rps33::MainForm();
    Application::Run(form);

    return 0;
}