#pragma once

namespace BSMakeTool {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for MakerTool
	/// </summary>
	public ref class MakerTool : public System::Windows::Forms::Form
	{
	public:
		MakerTool(void);

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MakerTool()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Panel^ Rendering_Panel;
	protected:

	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->Rendering_Panel = (gcnew System::Windows::Forms::Panel());
			this->SuspendLayout();
			// 
			// Rendering_Panel
			// 
			this->Rendering_Panel->Location = System::Drawing::Point(296, 13);
			this->Rendering_Panel->Name = L"Rendering_Panel";
			this->Rendering_Panel->Size = System::Drawing::Size(623, 493);
			this->Rendering_Panel->TabIndex = 0;
			// 
			// MakerTool
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 15);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1182, 698);
			this->Controls->Add(this->Rendering_Panel);
			this->Name = L"MakerTool";
			this->Text = L"MakerTool";
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
