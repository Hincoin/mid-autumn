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

            IniFile cfg = new IniFile("rmapshell.ini");
            string cfg_artist_resource_dir = cfg.IniReadValue("path","artist_res","");//String.Empty;
            string cfg_working_dir = cfg.IniReadValue("path","work_dir","");//String.Empty;
            this.WorkingDirText.Text = cfg_working_dir;
            this.ArtistDataResourceText.Text = cfg_artist_resource_dir;
        }

        public void GenerateBoudingBoxInfo()
        {
            string wdir = this.WorkingDirText.Text ;
            string artist_res = this.ArtistDataResourceText.Text;

            //todo: call the generate process
            //generate the bounding box info
        }
        public string GetArtistResPath() { return this.ArtistDataResourceText.Text; }
        public string GetWorkDir() { return this.WorkingDirText.Text; }
        private void PreProcessBtn_Click(object sender, EventArgs e)
        {
            GenerateBoudingBoxInfo();
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
                this.ArtistDataResourceText.Text = this.DirectoryfolderBrowserDialog.SelectedPath;
            }
        }
    }
}