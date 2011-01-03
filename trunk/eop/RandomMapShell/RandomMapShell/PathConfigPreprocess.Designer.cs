namespace RandomMapShell
{
    partial class PathConfigPreprocess
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.BrowseWorkingDir = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.WorkingDirText = new System.Windows.Forms.TextBox();
            this.ArtistDataResourceText = new System.Windows.Forms.TextBox();
            this.BrowserResourceDir = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.DirectoryfolderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            this.PreProcessBtn = new System.Windows.Forms.Button();
            this.OKBtn = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // BrowseWorkingDir
            // 
            this.BrowseWorkingDir.Location = new System.Drawing.Point(413, 61);
            this.BrowseWorkingDir.Name = "BrowseWorkingDir";
            this.BrowseWorkingDir.Size = new System.Drawing.Size(74, 23);
            this.BrowseWorkingDir.TabIndex = 0;
            this.BrowseWorkingDir.Text = "浏览";
            this.BrowseWorkingDir.UseVisualStyleBackColor = true;
            this.BrowseWorkingDir.Click += new System.EventHandler(this.BrowseWorkingDir_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 61);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(53, 12);
            this.label1.TabIndex = 1;
            this.label1.Text = "工作路径";
            // 
            // WorkingDirText
            // 
            this.WorkingDirText.Location = new System.Drawing.Point(79, 59);
            this.WorkingDirText.Name = "WorkingDirText";
            this.WorkingDirText.Size = new System.Drawing.Size(315, 21);
            this.WorkingDirText.TabIndex = 2;
            // 
            // ArtistDataResourceText
            // 
            this.ArtistDataResourceText.Location = new System.Drawing.Point(91, 95);
            this.ArtistDataResourceText.Name = "ArtistDataResourceText";
            this.ArtistDataResourceText.Size = new System.Drawing.Size(316, 21);
            this.ArtistDataResourceText.TabIndex = 3;
            // 
            // BrowserResourceDir
            // 
            this.BrowserResourceDir.Location = new System.Drawing.Point(414, 95);
            this.BrowserResourceDir.Name = "BrowserResourceDir";
            this.BrowserResourceDir.Size = new System.Drawing.Size(73, 18);
            this.BrowserResourceDir.TabIndex = 4;
            this.BrowserResourceDir.Text = "浏览";
            this.BrowserResourceDir.UseVisualStyleBackColor = true;
            this.BrowserResourceDir.Click += new System.EventHandler(this.BrowserResourceDir_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 98);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(77, 12);
            this.label2.TabIndex = 5;
            this.label2.Text = "美术资源路径";
            // 
            // PreProcessBtn
            // 
            this.PreProcessBtn.Location = new System.Drawing.Point(12, 139);
            this.PreProcessBtn.Name = "PreProcessBtn";
            this.PreProcessBtn.Size = new System.Drawing.Size(100, 32);
            this.PreProcessBtn.TabIndex = 6;
            this.PreProcessBtn.Text = "美术模型预处理";
            this.PreProcessBtn.UseVisualStyleBackColor = true;
            this.PreProcessBtn.Click += new System.EventHandler(this.PreProcessBtn_Click);
            // 
            // OKBtn
            // 
            this.OKBtn.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.OKBtn.Location = new System.Drawing.Point(414, 315);
            this.OKBtn.Name = "OKBtn";
            this.OKBtn.Size = new System.Drawing.Size(73, 27);
            this.OKBtn.TabIndex = 7;
            this.OKBtn.Text = "确定";
            this.OKBtn.UseVisualStyleBackColor = true;
            // 
            // PathConfigPreprocess
            // 
            this.AcceptButton = this.OKBtn;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(529, 389);
            this.Controls.Add(this.OKBtn);
            this.Controls.Add(this.PreProcessBtn);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.BrowserResourceDir);
            this.Controls.Add(this.ArtistDataResourceText);
            this.Controls.Add(this.WorkingDirText);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.BrowseWorkingDir);
            this.Name = "PathConfigPreprocess";
            this.Text = "PathConfigPreprocess";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button BrowseWorkingDir;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox WorkingDirText;
        private System.Windows.Forms.TextBox ArtistDataResourceText;
        private System.Windows.Forms.Button BrowserResourceDir;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.FolderBrowserDialog DirectoryfolderBrowserDialog;
        private System.Windows.Forms.Button PreProcessBtn;
        private System.Windows.Forms.Button OKBtn;
    }
}