namespace RandomMapShell
{
    partial class ARSResources
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
            this.ARSResourcetreeView = new System.Windows.Forms.TreeView();
            this.SuspendLayout();
            // 
            // ARSResourcetreeView
            // 
            this.ARSResourcetreeView.Location = new System.Drawing.Point(12, 12);
            this.ARSResourcetreeView.Name = "ARSResourcetreeView";
            this.ARSResourcetreeView.Size = new System.Drawing.Size(296, 426);
            this.ARSResourcetreeView.TabIndex = 0;
            // 
            // ARSResources
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(320, 450);
            this.Controls.Add(this.ARSResourcetreeView);
            this.Name = "ARSResources";
            this.Text = "ARSResources";
            this.Load += new System.EventHandler(this.ARSResources_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TreeView ARSResourcetreeView;
    }
}