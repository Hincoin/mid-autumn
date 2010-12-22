using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace RandomMapShell
{
    public partial class RMapShell : Form
    {
        string cfg_artist_resource_dir;
        string cfg_working_dir;//exe path

        string tex_model_resource;
        string water_tex_resource;
 
        public RMapShell()
        {
            InitializeComponent();
            //default setting
            cfg_artist_resource_dir = "e:\\work\\artist\\res";//String.Empty;
            cfg_working_dir= "E:\\work\\program\\bin\\Release";//String.Empty;
        }

        private void RMapShell_Load(object sender, EventArgs e)
        {

        }

        private void LoadFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(!can_work()) return;

            this.openRMFileDialog.InitialDirectory = cfg_artist_resource_dir + "\\map"; 
            this.openRMFileDialog.ShowDialog();
           
            string file_name = this.openRMFileDialog.FileName;
            IniFile ini_file = new IniFile(file_name);
            tex_model_resource = ini_file.IniReadValue("resources","decorators");
            string wall = ini_file.IniReadValue("resources","wall");
            string plant = ini_file.IniReadValue("resources","plant");
            string texture = ini_file.IniReadValue("resources","texture");
            string stone = ini_file.IniReadValue("resources","stone");
            if(tex_model_resource != wall ||
                tex_model_resource != plant ||
                tex_model_resource != stone ||
                tex_model_resource != texture)
            {
                MessageBox.Show(this, "资源路径不一致，你遇到了老的配置文件，暂时不支持！我很抱歉啊！");
                return ;
            }
            water_tex_resource = ini_file.IniReadValue("resources","water");

            this.WaterResFileText.Text = water_tex_resource;
            this.ResourceSetText.Text = tex_model_resource;
            //简化参数，统一设置，不支持时间变化
            string sun_color = ini_file.IniReadValue("mapinfo", "sceneInfo[0].dwSunColor");
            this.SunColorBtn.BackColor = HexToColor(sun_color);
            this.EnvColorBtn.BackColor = HexToColor(ini_file.IniReadValue("mapinfo", "sceneInfo[0].dwAmbientColor"));
            this.FogColorBtn.BackColor = HexToColor(ini_file.IniReadValue("mapinfo", "sceneInfo[0].dwFogColor"));
            this.FogNearText.Text = ini_file.IniReadValue("mapinfo", "sceneInfo[0].fFogStart");
            this.FogFarText.Text =  ini_file.IniReadValue("mapinfo", "sceneInfo[0].fFogEnd");
            this.SceneMusicText.Text = ini_file.IniReadValue("Music", "SceneMusic");
        }
        private bool can_work()
        {
            if (cfg_artist_resource_dir.Equals(""))
            {
                MessageBox.Show(this, "请配置工作路径");
                return false;
            }
            return true;
        }
        private Color HexToColor(string str)
        {
            Int32 x = Convert.ToInt32(str,16);
            int r =  ((x & 0x00ff0000) >> 16);
            int g = ((x & 0x0000ff00) >> 8 );
            int b = ((x & 0x000000ff));
            return Color.FromArgb(r, g, b);
        }

        private void Barrier_Click(object sender, EventArgs e)
        {

        }
    }
}