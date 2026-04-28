#pragma once
#include <vector>
#include <string>
#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>
#include "ShakerSort.h"
#include "Database.h"

using namespace msclr::interop;

namespace rps33 {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Drawing2D;
	using namespace System::IO;
	using namespace System::Collections::Generic;
	using namespace System::Runtime::InteropServices;

	public ref class RoundedPanel : public Panel
	{
	public:
		RoundedPanel()
		{
			this->DoubleBuffered = true;
			// pastel "card" color (not white)
			this->BackColor = Color::FromArgb(233, 242, 255);
			this->BorderColor = Color::FromArgb(191, 219, 254);
			this->CornerRadius = 16;
			this->BorderThickness = 1;
		}

		property int CornerRadius;
		property int BorderThickness;
		property Color BorderColor;

	protected:
		virtual void OnPaintBackground(PaintEventArgs^ e) override
		{
			// prevent default flickery background
		}

		virtual void OnPaint(PaintEventArgs^ e) override
		{
			Panel::OnPaint(e);
			e->Graphics->SmoothingMode = SmoothingMode::AntiAlias;

			System::Drawing::Rectangle rect = System::Drawing::Rectangle(0, 0, this->Width - 1, this->Height - 1);
			int r = Math::Max(0, CornerRadius);

			GraphicsPath^ path = gcnew GraphicsPath();
			int d = r * 2;
			if (d > 0) {
				path->AddArc((float)rect.X, (float)rect.Y, (float)d, (float)d, 180.0f, 90.0f);
				path->AddArc((float)(rect.Right - d), (float)rect.Y, (float)d, (float)d, 270.0f, 90.0f);
				path->AddArc((float)(rect.Right - d), (float)(rect.Bottom - d), (float)d, (float)d, 0.0f, 90.0f);
				path->AddArc((float)rect.X, (float)(rect.Bottom - d), (float)d, (float)d, 90.0f, 90.0f);
				path->CloseFigure();
			}
			else {
				path->AddRectangle(rect);
			}

			SolidBrush^ bg = gcnew SolidBrush(this->BackColor);
			e->Graphics->FillPath(bg, path);

			if (BorderThickness > 0) {
				Pen^ pen = gcnew Pen(BorderColor, (float)BorderThickness);
				e->Graphics->DrawPath(pen, path);
			}
		}
	};

	public ref class MainForm : public System::Windows::Forms::Form
	{
	public:
		MainForm(void)
		{
			InitializeComponent();
			//инициализация данных
			currentArray = gcnew System::Collections::Generic::List<int>();
		lastOriginalArray = gcnew System::Collections::Generic::List<int>();
		lastOperationWasSort = false;
		currentUsername = "";

			//инициализация sqlite базы данных
			try {
				//создание объекта базы данных
				db = new ::Database("sorting.db");  //глобальное пространство имен

				if (db->open()) {
					db->createTables();
					lblStatus->Text = "База данных SQLite подключена. Готов к работе.";
					lblStatus->ForeColor = Color::Green;
				}
				else {
					lblStatus->Text = "Ошибка подключения к БД SQLite!";
					lblStatus->ForeColor = Color::Red;
				}
			}
			catch (Exception^ ex) {
				lblStatus->Text = "Ошибка инициализации БД: " + ex->Message;
				lblStatus->ForeColor = Color::Red;
			}
			catch (...) {
				lblStatus->Text = "Неизвестная ошибка инициализации БД!";
				lblStatus->ForeColor = Color::Red;
			}
		}

	protected:
		virtual void OnPaintBackground(PaintEventArgs^ e) override
		{
			// subtle gradient background (modern look)
			System::Drawing::Rectangle r = this->ClientRectangle;
			if (r.Width <= 0 || r.Height <= 0) return;

			e->Graphics->SmoothingMode = SmoothingMode::HighQuality;
			// creative, noticeable background: diagonal pastel gradient
			LinearGradientBrush^ brush = gcnew LinearGradientBrush(
				r,
				Color::FromArgb(219, 234, 254),  // light blue
				Color::FromArgb(254, 226, 226),  // light rose
				LinearGradientMode::ForwardDiagonal
			);
			brush->WrapMode = WrapMode::TileFlipXY;
			e->Graphics->FillRectangle(brush, r);
		}
		//освободить все используемые ресурсы
		~MainForm()
		{
			//закрываем базу данных
			if (db != nullptr) {
				db->close();
				delete db;
				db = nullptr;
			}

			if (components)
			{
				delete components;
			}
		}
	private: RoundedPanel^ groupAuth;
	private: RoundedPanel^ groupArray;
	private: RoundedPanel^ groupHistory;
	private: System::Windows::Forms::TextBox^ txtPassword;
	private: System::Windows::Forms::TextBox^ txtLogin;
	private: System::Windows::Forms::RichTextBox^ txtArray;
	private: System::Windows::Forms::Button^ btnRegister;
	private: System::Windows::Forms::Button^ btnLogin;
	private: System::Windows::Forms::Button^ btnLogout;
	private: System::Windows::Forms::Button^ btnClearHistory;
	private: System::Windows::Forms::CheckBox^ chkShowPassword;
	private: System::Windows::Forms::ToolTip^ toolTip1;
	private: System::Windows::Forms::Button^ btnSave;
	private: System::Windows::Forms::Button^ btnSort;
	private: System::Windows::Forms::Button^ btnGenerate;
	private: System::Windows::Forms::Label^ labelLogin;
	private: System::Windows::Forms::Button^ btnHelp;
	
	private: System::Windows::Forms::Label^ labelPassword;
	private: System::Windows::Forms::Label^ labelArray;
	private: System::Windows::Forms::Label^ labelHistory;
	private: System::Windows::Forms::ListBox^ listHistory;
	private: System::Windows::Forms::Label^ lblStatus;
	private: System::Windows::Forms::Label^ lblAuthTitle;
	private: System::Windows::Forms::Label^ lblArrayTitle;
	private: System::Windows::Forms::Label^ lblHistoryTitle;

		   //данные программы
	private: System::Collections::Generic::List<int>^ currentArray;
	private: System::Collections::Generic::List<int>^ lastOriginalArray;
	private: bool lastOperationWasSort;
	private: System::String^ currentUsername;
	private: ::Database* db;  //указатель на нашу SQLite базу данных

	private:
		//обязательная переменная конструктора
		System::ComponentModel::Container^ components;

		//вспомогательная функция для преобразования String^ в std::string
	private:
		std::string ConvertToString(System::String^ s) {
			if (s == nullptr || s->Length == 0)
				return "";

			const char* chars = (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
			std::string result = chars;
			Marshal::FreeHGlobal(System::IntPtr((void*)chars));
			return result;
		}

		//функция для преобразования std::string в String^
	private:
		String^ ConvertToManagedString(const std::string& s) {
			if (s.empty()) return "";
			array<Byte>^ bytes = gcnew array<Byte>((int)s.size());
			Marshal::Copy((IntPtr)(void*)s.data(), bytes, 0, (int)s.size());
			return System::Text::Encoding::UTF8->GetString(bytes);
		}

		// вспомогательная функция: парсит txtArray в std::vector<int>
		std::vector<int> ParseTextToVector()
		{
			std::vector<int> out;
			array<Char>^ seps = gcnew array<Char>{' ', '\t', '\r', '\n', ',', ';'};
			array<String^>^ parts = txtArray->Text->Trim()->Split(seps, StringSplitOptions::RemoveEmptyEntries);
			for each (String ^ part in parts) {
				if (part->Length > 0) {
					try { out.push_back(Convert::ToInt32(part)); }
					catch (Exception^) { }
				}
			}
			return out;
		}

		// вспомогательная функция: обновляет txtArray и currentArray из вектора
		void UpdateArrayDisplayAndCurrent(const std::vector<int>& vec)
		{
			txtArray->Clear();
			currentArray->Clear();
			for (int v : vec) { txtArray->AppendText(v.ToString() + " "); currentArray->Add(v); }
		}

#pragma region Windows Form Designer generated code

		void InitializeComponent(void)
		{
			this->groupAuth = (gcnew RoundedPanel());
			this->labelPassword = (gcnew System::Windows::Forms::Label());
			this->labelLogin = (gcnew System::Windows::Forms::Label());
			this->lblAuthTitle = (gcnew System::Windows::Forms::Label());
			this->btnRegister = (gcnew System::Windows::Forms::Button());
			this->btnLogin = (gcnew System::Windows::Forms::Button());
			this->btnLogout = (gcnew System::Windows::Forms::Button());
			this->btnClearHistory = (gcnew System::Windows::Forms::Button());
			this->chkShowPassword = (gcnew System::Windows::Forms::CheckBox());
			this->txtPassword = (gcnew System::Windows::Forms::TextBox());
			this->txtLogin = (gcnew System::Windows::Forms::TextBox());
			this->toolTip1 = (gcnew System::Windows::Forms::ToolTip());
			this->btnHelp = (gcnew System::Windows::Forms::Button());
			this->groupArray = (gcnew RoundedPanel());
			this->lblArrayTitle = (gcnew System::Windows::Forms::Label());
			this->labelArray = (gcnew System::Windows::Forms::Label());
			this->btnSave = (gcnew System::Windows::Forms::Button());
			this->btnSort = (gcnew System::Windows::Forms::Button());
			this->btnGenerate = (gcnew System::Windows::Forms::Button());
			this->txtArray = (gcnew System::Windows::Forms::RichTextBox());
			this->groupHistory = (gcnew RoundedPanel());
			this->lblHistoryTitle = (gcnew System::Windows::Forms::Label());
			this->listHistory = (gcnew System::Windows::Forms::ListBox());
			this->labelHistory = (gcnew System::Windows::Forms::Label());
			this->lblStatus = (gcnew System::Windows::Forms::Label());
			this->groupAuth->SuspendLayout();
			this->groupArray->SuspendLayout();
			this->groupHistory->SuspendLayout();
			this->SuspendLayout();
			// 
			// groupAuth
			// 
			this->groupAuth->Controls->Add(this->lblAuthTitle);
			this->groupAuth->Controls->Add(this->labelPassword);
			this->groupAuth->Controls->Add(this->labelLogin);
			this->groupAuth->Controls->Add(this->btnRegister);
			this->groupAuth->Controls->Add(this->btnLogin);
			this->groupAuth->Controls->Add(this->btnLogout);
			this->groupAuth->Controls->Add(this->btnClearHistory);
			this->groupAuth->Controls->Add(this->chkShowPassword);
			this->groupAuth->Controls->Add(this->txtPassword);
			this->groupAuth->Controls->Add(this->txtLogin);
			this->groupAuth->Location = System::Drawing::Point(15, 15);
			this->groupAuth->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->groupAuth->Name = L"groupAuth";
			this->groupAuth->Padding = System::Windows::Forms::Padding(16, 18, 16, 16);
			this->groupAuth->Size = System::Drawing::Size(990, 108);
			this->groupAuth->TabIndex = 0;
			this->groupAuth->BackColor = System::Drawing::Color::FromArgb(233, 242, 255);
			// 
			// lblAuthTitle
			// 
			this->lblAuthTitle->AutoSize = true;
			this->lblAuthTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.0f, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->lblAuthTitle->ForeColor = System::Drawing::Color::FromArgb(30, 41, 59);
			this->lblAuthTitle->Location = System::Drawing::Point(16, 8);
			this->lblAuthTitle->Name = L"lblAuthTitle";
			this->lblAuthTitle->Size = System::Drawing::Size(122, 19);
			this->lblAuthTitle->TabIndex = 100;
			this->lblAuthTitle->Text = L"Авторизация";
			this->groupAuth->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(55)),
				static_cast<System::Int32>(static_cast<System::Byte>(65)), static_cast<System::Int32>(static_cast<System::Byte>(81)));
			// 
			// labelPassword
			// 
			this->labelPassword->AutoSize = true;
			this->labelPassword->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(71)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(105)));
			this->labelPassword->Location = System::Drawing::Point(300, 38);
			this->labelPassword->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
			this->labelPassword->Name = L"labelPassword";
			this->labelPassword->Size = System::Drawing::Size(71, 20);
			this->labelPassword->TabIndex = 5;
			this->labelPassword->Text = L"Пароль:";
			// 
			// labelLogin
			// 
			this->labelLogin->AutoSize = true;
			this->labelLogin->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(71)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(105)));
			this->labelLogin->Location = System::Drawing::Point(22, 38);
			this->labelLogin->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
			this->labelLogin->Name = L"labelLogin";
			this->labelLogin->Size = System::Drawing::Size(59, 20);
			this->labelLogin->TabIndex = 4;
			this->labelLogin->Text = L"Логин:";
			// 
			// btnRegister
			// 
			this->btnRegister->Location = System::Drawing::Point(735, 32);
			this->btnRegister->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->btnRegister->Name = L"btnRegister";
			this->btnRegister->Size = System::Drawing::Size(150, 38);
			this->btnRegister->TabIndex = 3;
			this->btnRegister->Text = L"Регистрация";
			this->btnRegister->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnRegister->FlatAppearance->BorderSize = 0;
			this->btnRegister->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(226)),
				static_cast<System::Int32>(static_cast<System::Byte>(232)), static_cast<System::Int32>(static_cast<System::Byte>(240)));
			this->btnRegister->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(30)),
				static_cast<System::Int32>(static_cast<System::Byte>(41)), static_cast<System::Int32>(static_cast<System::Byte>(59)));
			this->btnRegister->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(210)),
				static_cast<System::Int32>(static_cast<System::Byte>(220)), static_cast<System::Int32>(static_cast<System::Byte>(235)));
			this->btnRegister->UseVisualStyleBackColor = true;
			this->btnRegister->Click += gcnew System::EventHandler(this, &MainForm::btnRegister_Click);
			this->toolTip1->SetToolTip(this->btnRegister, L"Создать новый аккаунт");
			// 
			// btnLogin
			// 
			this->btnLogin->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(123)),
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->btnLogin->ForeColor = System::Drawing::SystemColors::ButtonHighlight;
			this->btnLogin->Location = System::Drawing::Point(600, 32);
			this->btnLogin->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->btnLogin->Name = L"btnLogin";
			this->btnLogin->Size = System::Drawing::Size(120, 38);
			this->btnLogin->TabIndex = 2;
			this->btnLogin->Text = L"Войти";
			this->btnLogin->UseVisualStyleBackColor = false;
			this->btnLogin->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnLogin->FlatAppearance->BorderSize = 0;
			this->btnLogin->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)),
				static_cast<System::Int32>(static_cast<System::Byte>(105)), static_cast<System::Int32>(static_cast<System::Byte>(217)));
			this->btnLogin->Click += gcnew System::EventHandler(this, &MainForm::btnLogin_Click);
			this->toolTip1->SetToolTip(this->btnLogin, L"Войти в аккаунт");
			// 
			// btnLogout
			// 
			this->btnLogout->BackColor = System::Drawing::Color::FromArgb(239, 68, 68);
			this->btnLogout->ForeColor = System::Drawing::Color::White;
			this->btnLogout->Location = System::Drawing::Point(895, 32);
			this->btnLogout->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->btnLogout->Name = L"btnLogout";
			this->btnLogout->Size = System::Drawing::Size(80, 38);
			this->btnLogout->TabIndex = 6;
			this->btnLogout->Text = L"Выйти";
			this->btnLogout->UseVisualStyleBackColor = false;
			this->btnLogout->Enabled = false;
			this->btnLogout->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnLogout->FlatAppearance->BorderSize = 0;
			this->btnLogout->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(220, 38, 38);
			this->btnLogout->Click += gcnew System::EventHandler(this, &MainForm::btnLogout_Click);
			this->toolTip1->SetToolTip(this->btnLogout, L"Выйти и сменить пользователя");
			// 
			// btnClearHistory
			// 
			this->btnClearHistory->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(244)), static_cast<System::Int32>(static_cast<System::Byte>(63)),
				static_cast<System::Int32>(static_cast<System::Byte>(94)));
			this->btnClearHistory->ForeColor = System::Drawing::Color::White;
			this->btnClearHistory->Location = System::Drawing::Point(600, 74);
			this->btnClearHistory->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->btnClearHistory->Name = L"btnClearHistory";
			this->btnClearHistory->Size = System::Drawing::Size(285, 28);
			this->btnClearHistory->TabIndex = 8;
			this->btnClearHistory->Text = L"Очистить историю";
			this->btnClearHistory->UseVisualStyleBackColor = false;
			this->btnClearHistory->Enabled = false;
			this->btnClearHistory->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnClearHistory->FlatAppearance->BorderSize = 0;
			this->btnClearHistory->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(225)),
				static_cast<System::Int32>(static_cast<System::Byte>(29)), static_cast<System::Int32>(static_cast<System::Byte>(72)));
			this->btnClearHistory->Click += gcnew System::EventHandler(this, &MainForm::btnClearHistory_Click);
			this->toolTip1->SetToolTip(this->btnClearHistory, L"Удалить все сохранения текущего пользователя");
			// 
			// chkShowPassword
			// 
			this->chkShowPassword->AutoSize = true;
			this->chkShowPassword->Location = System::Drawing::Point(390, 68);
			this->chkShowPassword->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->chkShowPassword->Name = L"chkShowPassword";
			this->chkShowPassword->Size = System::Drawing::Size(157, 24);
			this->chkShowPassword->TabIndex = 7;
			this->chkShowPassword->Text = L"Показать пароль";
			this->chkShowPassword->UseVisualStyleBackColor = true;
			this->chkShowPassword->CheckedChanged += gcnew System::EventHandler(this, &MainForm::chkShowPassword_CheckedChanged);
			// 
			// txtPassword
			// 
			this->txtPassword->Location = System::Drawing::Point(390, 34);
			this->txtPassword->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->txtPassword->Name = L"txtPassword";
			this->txtPassword->Size = System::Drawing::Size(178, 26);
			this->txtPassword->TabIndex = 1;
			this->txtPassword->BackColor = System::Drawing::Color::FromArgb(248, 250, 252);
			this->txtPassword->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->txtPassword->UseSystemPasswordChar = true;
			this->toolTip1->SetToolTip(this->txtPassword, L"Пароль");
			// 
			// txtLogin
			// 
			this->txtLogin->Location = System::Drawing::Point(105, 34);
			this->txtLogin->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->txtLogin->Name = L"txtLogin";
			this->txtLogin->Size = System::Drawing::Size(148, 26);
			this->txtLogin->TabIndex = 0;
			this->txtLogin->BackColor = System::Drawing::Color::FromArgb(248, 250, 252);
			this->txtLogin->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->toolTip1->SetToolTip(this->txtLogin, L"Логин (уникальный)");
			// 
			// btnHelp
			// 
			this->btnHelp->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(52)), static_cast<System::Int32>(static_cast<System::Byte>(152)),
				static_cast<System::Int32>(static_cast<System::Byte>(219)));
			this->btnHelp->ForeColor = System::Drawing::Color::White;
			this->btnHelp->Location = System::Drawing::Point(800, 784);
			this->btnHelp->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->btnHelp->Name = L"btnHelp";
			this->btnHelp->Size = System::Drawing::Size(180, 38);
			this->btnHelp->TabIndex = 6;
			this->btnHelp->Text = L"Справка";
			this->btnHelp->UseVisualStyleBackColor = false;
			this->btnHelp->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnHelp->FlatAppearance->BorderSize = 0;
			this->btnHelp->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(37)),
				static_cast<System::Int32>(static_cast<System::Byte>(99)), static_cast<System::Int32>(static_cast<System::Byte>(235)));
			this->btnHelp->Click += gcnew System::EventHandler(this, &MainForm::btnHelp_Click);
			// 
			// groupArray
			// 
			this->groupArray->Controls->Add(this->lblArrayTitle);
			this->groupArray->Controls->Add(this->labelArray);
			this->groupArray->Controls->Add(this->btnSave);
			this->groupArray->Controls->Add(this->btnSort);
			this->groupArray->Controls->Add(this->btnGenerate);
			this->groupArray->Controls->Add(this->txtArray);
			this->groupArray->Location = System::Drawing::Point(15, 138);
			this->groupArray->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->groupArray->Name = L"groupArray";
			this->groupArray->Padding = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->groupArray->Size = System::Drawing::Size(990, 277);
			this->groupArray->TabIndex = 1;
			this->groupArray->BackColor = System::Drawing::Color::FromArgb(233, 242, 255);
			this->groupArray->Padding = System::Windows::Forms::Padding(16, 18, 16, 16);
			// 
			// lblArrayTitle
			// 
			this->lblArrayTitle->AutoSize = true;
			this->lblArrayTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.0f, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->lblArrayTitle->ForeColor = System::Drawing::Color::FromArgb(30, 41, 59);
			this->lblArrayTitle->Location = System::Drawing::Point(16, 8);
			this->lblArrayTitle->Name = L"lblArrayTitle";
			this->lblArrayTitle->Size = System::Drawing::Size(141, 19);
			this->lblArrayTitle->TabIndex = 101;
			this->lblArrayTitle->Text = L"Работа с массивом";
			this->groupArray->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(55)),
				static_cast<System::Int32>(static_cast<System::Byte>(65)), static_cast<System::Int32>(static_cast<System::Byte>(81)));
			// 
			// labelArray
			// 
			this->labelArray->AutoSize = true;
			this->labelArray->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.0f, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->labelArray->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(30)),
				static_cast<System::Int32>(static_cast<System::Byte>(41)), static_cast<System::Int32>(static_cast<System::Byte>(59)));
			this->labelArray->Location = System::Drawing::Point(22, 38);
			this->labelArray->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
			this->labelArray->Name = L"labelArray";
			this->labelArray->Size = System::Drawing::Size(229, 20);
			this->labelArray->TabIndex = 6;
			this->labelArray->Text = L"Массив (через пробел):";
			// 
			// btnSave
			// 
			this->btnSave->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(193)),
				static_cast<System::Int32>(static_cast<System::Byte>(7)));
			this->btnSave->ForeColor = System::Drawing::Color::Black;
			this->btnSave->Location = System::Drawing::Point(412, 208);
			this->btnSave->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->btnSave->Name = L"btnSave";
			this->btnSave->Size = System::Drawing::Size(180, 46);
			this->btnSave->TabIndex = 5;
			this->btnSave->Text = L"Сохранить в БД";
			this->btnSave->UseVisualStyleBackColor = false;
			this->btnSave->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnSave->FlatAppearance->BorderSize = 0;
			this->btnSave->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(234)),
				static_cast<System::Int32>(static_cast<System::Byte>(179)), static_cast<System::Int32>(static_cast<System::Byte>(8)));
			this->btnSave->Click += gcnew System::EventHandler(this, &MainForm::btnSave_Click);
			// 
			// btnSort
			// 
			this->btnSort->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(111)), static_cast<System::Int32>(static_cast<System::Byte>(66)),
				static_cast<System::Int32>(static_cast<System::Byte>(193)));
			this->btnSort->ForeColor = System::Drawing::Color::White;
			this->btnSort->Location = System::Drawing::Point(218, 208);
			this->btnSort->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->btnSort->Name = L"btnSort";
			this->btnSort->Size = System::Drawing::Size(180, 46);
			this->btnSort->TabIndex = 4;
			this->btnSort->Text = L"Сортировать";
			this->btnSort->UseVisualStyleBackColor = false;
			this->btnSort->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnSort->FlatAppearance->BorderSize = 0;
			this->btnSort->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(93)),
				static_cast<System::Int32>(static_cast<System::Byte>(53)), static_cast<System::Int32>(static_cast<System::Byte>(163)));
			this->btnSort->Click += gcnew System::EventHandler(this, &MainForm::btnSort_Click);
			// 
			// btnGenerate
			// 
			this->btnGenerate->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(40)), static_cast<System::Int32>(static_cast<System::Byte>(167)),
				static_cast<System::Int32>(static_cast<System::Byte>(69)));
			this->btnGenerate->ForeColor = System::Drawing::Color::White;
			this->btnGenerate->Location = System::Drawing::Point(22, 208);
			this->btnGenerate->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->btnGenerate->Name = L"btnGenerate";
			this->btnGenerate->Size = System::Drawing::Size(180, 46);
			this->btnGenerate->TabIndex = 3;
			this->btnGenerate->Text = L"Сгенерировать";
			this->btnGenerate->UseVisualStyleBackColor = false;
			this->btnGenerate->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnGenerate->FlatAppearance->BorderSize = 0;
			this->btnGenerate->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(33)),
				static_cast<System::Int32>(static_cast<System::Byte>(136)), static_cast<System::Int32>(static_cast<System::Byte>(56)));
			this->btnGenerate->Click += gcnew System::EventHandler(this, &MainForm::btnGenerate_Click);
			// 
			// txtArray
			// 
			this->txtArray->Location = System::Drawing::Point(22, 69);
			this->txtArray->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->txtArray->Name = L"txtArray";
			this->txtArray->Size = System::Drawing::Size(943, 121);
			this->txtArray->TabIndex = 2;
			this->txtArray->Text = L"5 3 8 1 2 9 4 7 6";
			this->txtArray->BackColor = System::Drawing::Color::FromArgb(248, 250, 252);
			// 
			// groupHistory
			// 
			this->groupHistory->Controls->Add(this->lblHistoryTitle);
			this->groupHistory->Controls->Add(this->listHistory);
			this->groupHistory->Controls->Add(this->labelHistory);
			this->groupHistory->Location = System::Drawing::Point(15, 431);
			this->groupHistory->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->groupHistory->Name = L"groupHistory";
			this->groupHistory->Padding = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->groupHistory->Size = System::Drawing::Size(990, 308);
			this->groupHistory->TabIndex = 2;
			this->groupHistory->BackColor = System::Drawing::Color::FromArgb(233, 242, 255);
			this->groupHistory->Padding = System::Windows::Forms::Padding(16, 18, 16, 16);
			// 
			// lblHistoryTitle
			// 
			this->lblHistoryTitle->AutoSize = true;
			this->lblHistoryTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.0f, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->lblHistoryTitle->ForeColor = System::Drawing::Color::FromArgb(30, 41, 59);
			this->lblHistoryTitle->Location = System::Drawing::Point(16, 8);
			this->lblHistoryTitle->Name = L"lblHistoryTitle";
			this->lblHistoryTitle->Size = System::Drawing::Size(169, 19);
			this->lblHistoryTitle->TabIndex = 102;
			this->lblHistoryTitle->Text = L"История (SQLite БД)";
			this->groupHistory->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(55)),
				static_cast<System::Int32>(static_cast<System::Byte>(65)), static_cast<System::Int32>(static_cast<System::Byte>(81)));
			// 
			// listHistory
			// 
			this->listHistory->FormattingEnabled = true;
			this->listHistory->ItemHeight = 20;
			this->listHistory->Location = System::Drawing::Point(22, 62);
			this->listHistory->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->listHistory->Name = L"listHistory";
			this->listHistory->Size = System::Drawing::Size(943, 224);
			this->listHistory->TabIndex = 1;
			this->listHistory->Font = (gcnew System::Drawing::Font(L"Consolas", 9.0f));
			this->listHistory->BackColor = System::Drawing::Color::FromArgb(248, 250, 252);
			// 
			// labelHistory
			// 
			this->labelHistory->AutoSize = true;
			this->labelHistory->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.0f, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->labelHistory->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(30)),
				static_cast<System::Int32>(static_cast<System::Byte>(41)), static_cast<System::Int32>(static_cast<System::Byte>(59)));
			this->labelHistory->Location = System::Drawing::Point(22, 31);
			this->labelHistory->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
			this->labelHistory->Name = L"labelHistory";
			this->labelHistory->Size = System::Drawing::Size(205, 20);
			this->labelHistory->TabIndex = 0;
			this->labelHistory->Text = L"История сохранений:";
			// 
			// lblStatus
			// 
			this->lblStatus->AutoSize = true;
			this->lblStatus->Location = System::Drawing::Point(22, 754);
			this->lblStatus->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
			this->lblStatus->Name = L"lblStatus";
			this->lblStatus->Size = System::Drawing::Size(145, 20);
			this->lblStatus->TabIndex = 4;
			this->lblStatus->Text = L"Статусная строка";
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(9, 20);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.0f, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->ClientSize = System::Drawing::Size(1026, 844);
			this->Controls->Add(this->lblStatus);
			this->Controls->Add(this->groupHistory);
			this->Controls->Add(this->btnHelp);
			this->Controls->Add(this->groupArray);
			this->Controls->Add(this->groupAuth);
			this->BackColor = System::Drawing::Color::FromArgb(219, 234, 254);
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Margin = System::Windows::Forms::Padding(4, 5, 4, 5);
			this->MaximumSize = System::Drawing::Size(1048, 900);
			this->MinimumSize = System::Drawing::Size(1048, 900);
			this->Name = L"MainForm";
			this->Text = L"Сортировщик массивов с SQLite БД";
			this->WindowState = System::Windows::Forms::FormWindowState::Maximized;
			this->AcceptButton = this->btnLogin;
			this->groupAuth->ResumeLayout(false);
			this->groupAuth->PerformLayout();
			this->groupArray->ResumeLayout(false);
			this->groupArray->PerformLayout();
			this->groupHistory->ResumeLayout(false);
			this->groupHistory->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

		//обработчики ошибок

	private: System::Void btnLogin_Click(System::Object^ sender, System::EventArgs^ e) {
		String^ login = txtLogin->Text;
		String^ password = txtPassword->Text;

		if (login->Length == 0 || password->Length == 0) {
			MessageBox::Show("Введите логин и пароль!", "Ошибка входа",
				MessageBoxButtons::OK, MessageBoxIcon::Warning);
			return;
		}

		if (db == nullptr) {
			MessageBox::Show("База данных не инициализирована!", "Ошибка",
				MessageBoxButtons::OK, MessageBoxIcon::Error);
			return;
		}

		std::string username = ConvertToString(login);
		std::string pass = ConvertToString(password);

		int userId = db->getUserId(username);
		if (userId != -1) {
			// пользователь найден, проверяем пароль
			if (db->checkUser(username, pass)) {
				currentUsername = login;
				lblStatus->Text = "Вход выполнен: " + login + " (SQLite)";
				lblStatus->ForeColor = Color::Green;

				// блокируем поля авторизации
				this->txtLogin->Enabled = false;
				this->txtPassword->Enabled = false;
				this->btnLogin->Enabled = false;
				this->btnRegister->Enabled = false;
					this->btnLogout->Enabled = true;
					this->btnClearHistory->Enabled = true;

					// чтобы визуально было понятно, что кнопки выключены
					this->btnLogin->BackColor = System::Drawing::Color::FromArgb(203, 213, 225);
					this->btnLogin->ForeColor = System::Drawing::Color::FromArgb(71, 85, 105);
					this->btnRegister->BackColor = System::Drawing::Color::FromArgb(203, 213, 225);
					this->btnRegister->ForeColor = System::Drawing::Color::FromArgb(71, 85, 105);

				LoadHistory();
			}
			else {
				MessageBox::Show("Неверный пароль.", "Ошибка входа",
					MessageBoxButtons::OK, MessageBoxIcon::Error);
			}
		}
		else {
			// пользователь не найден, предлагаем зарегистрироваться
			if (MessageBox::Show("Пользователь не найден. Зарегистрироваться?", "Вход",
				MessageBoxButtons::YesNo, MessageBoxIcon::Question) == Windows::Forms::DialogResult::Yes) {
				if (db->addUser(username, pass)) {
					currentUsername = login;
					lblStatus->Text = "Зарегистрирован и вошел: " + login;
					lblStatus->ForeColor = Color::Green;
					this->txtLogin->Enabled = false;
					this->txtPassword->Enabled = false;
					this->btnLogin->Enabled = false;
					this->btnRegister->Enabled = false;
					this->btnLogout->Enabled = true;
					this->btnClearHistory->Enabled = true;

					this->btnLogin->BackColor = System::Drawing::Color::FromArgb(203, 213, 225);
					this->btnLogin->ForeColor = System::Drawing::Color::FromArgb(71, 85, 105);
					this->btnRegister->BackColor = System::Drawing::Color::FromArgb(203, 213, 225);
					this->btnRegister->ForeColor = System::Drawing::Color::FromArgb(71, 85, 105);
					LoadHistory();
				}
				else {
					MessageBox::Show("Ошибка регистрации! Попробуйте другой логин.", "Ошибка",
						MessageBoxButtons::OK, MessageBoxIcon::Error);
				}
			}
		}
	}

	private: System::Void btnRegister_Click(System::Object^ sender, System::EventArgs^ e) {
		String^ login = txtLogin->Text;
		String^ password = txtPassword->Text;

		if (login->Length == 0 || password->Length == 0) {
			MessageBox::Show("Введите логин и пароль!", "Ошибка регистрации",
				MessageBoxButtons::OK, MessageBoxIcon::Warning);
			return;
		}

		if (db == nullptr) {
			MessageBox::Show("База данных не инициализирована!", "Ошибка",
				MessageBoxButtons::OK, MessageBoxIcon::Error);
			return;
		}

		std::string username = ConvertToString(login);
		std::string pass = ConvertToString(password);

		int userId = db->getUserId(username);
		if (userId != -1) {
			MessageBox::Show("Логин уже занят. Выберите другой логин или войдите.", "Ошибка регистрации",
				MessageBoxButtons::OK, MessageBoxIcon::Warning);
			return;
		}

		if (db->addUser(username, pass)) {
			MessageBox::Show("Пользователь '" + login + "' успешно зарегистрирован в SQLite БД!\n\n" +
				"Теперь вы можете войти в систему.",
				"Регистрация успешна",
				MessageBoxButtons::OK, MessageBoxIcon::Information);

			lblStatus->Text = "Зарегистрирован новый пользователь: " + login;
			lblStatus->ForeColor = Color::DarkOrange;
		}
		else {
			MessageBox::Show("Ошибка регистрации! Попробуйте другой логин.",
				"Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
		}
	}

	private: System::Void btnLogout_Click(System::Object^ sender, System::EventArgs^ e) {
		currentUsername = "";

		this->txtLogin->Enabled = true;
		this->txtPassword->Enabled = true;
		this->btnLogin->Enabled = true;
		this->btnRegister->Enabled = true;
		this->btnLogout->Enabled = false;
		this->btnClearHistory->Enabled = false;

		// визуально возвращаем "активный" вид кнопкам
		this->btnLogin->BackColor = System::Drawing::Color::FromArgb(0, 123, 255);
		this->btnLogin->ForeColor = System::Drawing::Color::White;
		this->btnRegister->BackColor = System::Drawing::Color::FromArgb(226, 232, 240);
		this->btnRegister->ForeColor = System::Drawing::Color::FromArgb(30, 41, 59);
		this->btnLogout->BackColor = System::Drawing::Color::FromArgb(108, 117, 125);
		this->btnLogout->ForeColor = System::Drawing::Color::White;

		this->txtPassword->Clear();

		listHistory->Items->Clear();
		listHistory->Items->Add("Войдите, чтобы увидеть историю.");

		lblStatus->Text = "Вы вышли из аккаунта. Можно войти снова.";
		lblStatus->ForeColor = Color::DarkOrange;
	}

	private: System::Void chkShowPassword_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
		this->txtPassword->UseSystemPasswordChar = !this->chkShowPassword->Checked;
	}

	private: System::Void btnClearHistory_Click(System::Object^ sender, System::EventArgs^ e) {
		if (currentUsername == nullptr || currentUsername->Length == 0) {
			MessageBox::Show("Сначала войдите в систему.", "История",
				MessageBoxButtons::OK, MessageBoxIcon::Information);
			return;
		}
		if (db == nullptr) return;

		if (MessageBox::Show("Удалить всю историю сохранений для текущего пользователя?",
			"Подтверждение",
			MessageBoxButtons::YesNo,
			MessageBoxIcon::Warning) != Windows::Forms::DialogResult::Yes) {
			return;
		}

		std::string username = ConvertToString(currentUsername);
		bool ok = db->clearUserHistory(username);
		if (!ok) {
			MessageBox::Show("Не удалось очистить историю.", "Ошибка",
				MessageBoxButtons::OK, MessageBoxIcon::Error);
			return;
		}

		listHistory->Items->Clear();
		listHistory->Items->Add("История очищена.");
		lblStatus->Text = "История пользователя очищена.";
		lblStatus->ForeColor = Color::Green;
	}


	private: System::Void btnHelp_Click(System::Object^ sender, System::EventArgs^ e) {
		String^ help = "Использование:\n";
		help += "1) Введите логин и пароль.\n";
		help += "2) Нажмите 'Войти' если у вас есть учетная запись.\n";
		help += "Если учетной записи нет, при попытке входа будет предложено зарегистрироваться.\n";
		help += "3) После входа вы сможете генерировать, сортировать и сохранять массивы в базу.\n";
		help += "4) Кнопка 'Сохранить в БД' сохраняет текущий массив для вошедшего пользователя.\n\n";
		

		MessageBox::Show(help, "Справка", MessageBoxButtons::OK, MessageBoxIcon::Information);
	}

	private: System::Void btnGenerate_Click(System::Object^ sender, System::EventArgs^ e) {
		Random^ rand = gcnew Random();
		int count = rand->Next(5, 25);

		std::vector<int> vec;
		vec.reserve(count);
		for (int i = 0; i < count; ++i) vec.push_back(rand->Next(0, 100));

	
		UpdateArrayDisplayAndCurrent(vec);

	
		lastOriginalArray->Clear();
		lastOperationWasSort = false;

		lblStatus->Text = String::Format("Сгенерировано {0} случайных чисел (0-99)", count);
		lblStatus->ForeColor = Color::Blue;
	}

	private: System::Void btnSort_Click(System::Object^ sender, System::EventArgs^ e) {
		std::vector<int> parsed = ParseTextToVector();
		if (parsed.empty()) {
			MessageBox::Show("Введите массив чисел!\n\nПример: 5 3 8 1 2",
				"Ошибка сортировки",
				MessageBoxButtons::OK, MessageBoxIcon::Error);
			return;
		}

		lastOriginalArray->Clear();
		for (int v : parsed) lastOriginalArray->Add(v);

		std::vector<int> sortedVec = parsed;
		ShakerSort::sort(sortedVec);

		
		UpdateArrayDisplayAndCurrent(sortedVec);
		lastOperationWasSort = true;

		lblStatus->Text = String::Format("Отсортировано {0} элементов (шейкерная сортировка)", parsed.size());
		lblStatus->ForeColor = Color::Purple;
	}

private: System::Void btnSave_Click(System::Object^ sender, System::EventArgs^ e) {
	if (currentUsername == nullptr || currentUsername->Length == 0) {
		MessageBox::Show("Сначала войдите в систему!", "Ошибка сохранения",
			MessageBoxButtons::OK, MessageBoxIcon::Warning);
		return;
	}

	if (db == nullptr) {
		MessageBox::Show("База данных не инициализирована!", "Ошибка",
			MessageBoxButtons::OK, MessageBoxIcon::Error);
		return;
	}

    std::vector<int> original, sorted;

    if (lastOperationWasSort && lastOriginalArray->Count > 0) {
        for each (int v in lastOriginalArray) original.push_back(v);
        for each (int v in currentArray) sorted.push_back(v);
    }
    else {
        original = ParseTextToVector();
        if (original.empty()) {
            MessageBox::Show("Нет данных для сохранения!\nСгенерируйте или введите массив.",
                "Ошибка сохранения",
                MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        sorted = original;
        ShakerSort::sort(sorted);
    }

	std::string username = ConvertToString(currentUsername);

	if (db->saveArray(username, original, sorted)) {
		//добавляем запись в историю на форме
		String^ historyItem = String::Format(
			"{0} - {1} элементов (оригинальный: {2}...{3})",
			DateTime::Now.ToString("HH:mm:ss"),
			(int)original.size(),
			original.front(),
			original.back()
		);
		listHistory->Items->Insert(0, historyItem);

		lblStatus->Text = "Массив успешно сохранен в SQLite БД";
		lblStatus->ForeColor = Color::Green;

		MessageBox::Show("Данные сохранены в SQLite базу данных!\n\n" +
			"Файл БД: sorting.db\n" +
			"Оригинальный массив: " + original.size() + " элементов\n" +
			"Отсортированный массив: " + sorted.size() + " элементов",
			"Сохранение успешно",
			MessageBoxButtons::OK, MessageBoxIcon::Information);

		//обновляем историю из БД
		LoadHistory();

			// reset sort tracking
			lastOriginalArray->Clear();
			lastOperationWasSort = false;
	}
	else {
		MessageBox::Show("Ошибка сохранения в БД SQLite!", "Ошибка",
			MessageBoxButtons::OK, MessageBoxIcon::Error);
	}
}



	private: void LoadHistory() {
		if (currentUsername == nullptr || currentUsername->Length == 0) return;
		if (db == nullptr) return;

		listHistory->Items->Clear();

		std::string username = ConvertToString(currentUsername);
		auto history = db->getUserArrays(username);

		if (history.empty()) {
			listHistory->Items->Add("История пуста (SQLite БД)");
			listHistory->Items->Add("Сохраните первый массив!");
			return;
		}

		//ограничиваем показ 20 последними записями
		int count = 0;
		for (const auto& entry : history) {
			if (count++ >= 20) break;
			String^ raw = ConvertToManagedString(entry);
			array<String^>^ parts = raw->Split('\t');
			if (parts->Length >= 3) {
				String^ date = parts[0];
				String^ original = parts[1];
				String^ sorted = parts[2];
				listHistory->Items->Add(String::Format("{0} | Исходный: {1} -> Отсортированный: {2}", date, original, sorted));
			}
			else {
				listHistory->Items->Add(raw);
			}
		}

		lblStatus->Text = String::Format("Загружено {0} записей из SQLite БД", history.size());
	}
	};
}