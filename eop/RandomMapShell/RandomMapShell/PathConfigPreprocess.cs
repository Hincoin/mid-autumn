using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace RandomMapShell
{
    public partial class PathConfigPreprocess : Form
    {
        public PathConfigPreprocess()
        {
            InitializeComponent();
        }

        private void PreProcessBtn_Click(object sender, EventArgs e)
        {
            //todo: call the generate process
        }

        private void BrowseWorkingDir_Click(object sender, EventArgs e)
        {
            if(this.DirectoryfolderBrowserDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.WorkingDirText.Text = this.DirectoryfolderBrowserDialog.SelectedPath;
            }
        }

        private void BrowserResourceDir_Click(object sender, EventArgs e)
        {
            if(this.DirectoryfolderBrowserDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.BrowserResourceDir.Text = this.DirectoryfolderBrowserDialog.SelectedPath;
            }
        }
    }
}