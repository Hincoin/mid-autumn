using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace RandomMapShell
{
    public partial class RMapShell : Form
    {
        string cfg_artist_resource_dir;
        string cfg_working_dir;//exe path

        string tex_model_resource;
        string water_tex_resource;
        string cur_edt_file;
        PathConfigPreprocess path_config_dlg;
        ARSResources ars_res_dlg;
 
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

        private void OnOpenFile(object sender, EventArgs e)
        {
            string file_name = this.openRMFileDialog.FileName;
            if(!File.Exists(file_name)) return ;
            cur_edt_file = file_name;
            IniFile ini_file = new IniFile(cur_edt_file);
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

            //resource setting
            this.WaterResFileText.Text = water_tex_resource;
            this.ResourceSetText.Text = tex_model_resource;
            //environment setting
            //简化参数，统一设置，不支持时间变化，要是不嫌参数多的话可以支持一下
            string sun_color = ini_file.IniReadValue("mapinfo", "sceneInfo[0].dwSunColor");
            this.SunColorBtn.BackColor = HexToColor(sun_color);
            this.EnvColorBtn.BackColor = HexToColor(ini_file.IniReadValue("mapinfo", "sceneInfo[0].dwAmbientColor"));
            this.FogColorBtn.BackColor = HexToColor(ini_file.IniReadValue("mapinfo", "sceneInfo[0].dwFogColor"));
            this.FogNearText.Text = ini_file.IniReadValue("mapinfo", "sceneInfo[0].fFogStart");
            this.FogFarText.Text =  ini_file.IniReadValue("mapinfo", "sceneInfo[0].fFogEnd");
            this.SceneMusicText.Text = ini_file.IniReadValue("Music", "SceneMusic");

            //water
            this.WaterHeightText.Text = ini_file.IniReadValue("WaterParameters","height");
            string refl_str = ini_file.IniReadValue("WaterParameters", "reflection","0");
            if(!refl_str.Equals(""))
                this.WaterReflectionCmb.SelectedIndex = Convert.ToInt32(refl_str);
            bool has_water = 
                ini_file.IniReadValue("MethodKind", "WaterKind", "NULL") == "AllWater" ||
                ini_file.IniReadValue("MethodKind","WaterKind","NULL") == "default";
            this.ChkHasWater.Checked =  has_water;
            if (!has_water)
                this.WaterParameterGroupBox.Hide();
            this.ChkHasWater.CheckedChanged += OnWaterChecked;

            string wave_length = ini_file.IniReadValue("mapinfo","waveInfo.WavePhysicInfo.nWaveSize","200");
            string wave_period = ini_file.IniReadValue("mapinfo","nWaveLife","1500");
            string wave_density = ini_file.IniReadValue("mapinfo","waveInfo.WavePhysicInfo.nWavePerGrid","2");
            string water_transparent_height = ini_file.IniReadValue("WaterParameters","depth","0");
            this.WaveDensityText.Text = wave_density;
            this.WaveLengthText.Text = wave_length;
            this.WavePeriodText.Text = wave_period;
            this.WaterTransparentHeightText.Text = water_transparent_height;

            //ground setting
            string texture_kind = ini_file.IniReadValue("MethodKind","RoadKind");
            if(texture_kind.Equals("DefaultTexture"))//2 
            {
                this.CmbTextureMethod.SelectedIndex = 1;
            }
            else if(texture_kind.Equals("NatureTexture"))
            {
                this.CmbTextureMethod.SelectedIndex = 0;
            }
            //color
            this.PathColorBtn.BackColor = RGB565ToColor(ini_file.IniReadValue("GroundParameters","CentralColor"));
            this.PathBorderColorBtn.BackColor=RGB565ToColor(ini_file.IniReadValue("GroundParameters","BorderColor"));
            this.BarrierColorBtn.BackColor=RGB565ToColor(ini_file.IniReadValue("GroundParameters","BarrierColor"));
            //test
            string clr_str = ColorToRGB565(this.PathBorderColorBtn.BackColor);
            //height
            string height_kind = ini_file.IniReadValue("MethodKind","GroundKind");
            if(height_kind.Equals("Cayon"))
                this.CmbGroundHeight.SelectedIndex = 0;
            else if(height_kind.Equals("Smooth"))
                this.CmbGroundHeight.SelectedIndex = 1;
            else if(height_kind.Equals("Flat"))
                this.CmbGroundHeight.SelectedIndex = 2;
            if(this.CmbGroundHeight.SelectedIndex == 0 || 
                this.CmbGroundHeight.SelectedIndex == 1)
            {
                this.CayonOrSmoothGroupBox.Show();
            }
            else
                this.CayonOrSmoothGroupBox.Hide();

            this.CmbGroundHeight.SelectedIndexChanged += OnGroundHeightMethodChanged;
            string out_most_barrier_height = ini_file.IniReadValue("GroundParameters","OutBarrierHeight","0");
            string path_height = ini_file.IniReadValue("GroundParameters","GroundBaseHeight","0");
            string inner_barrier_height = ini_file.IniReadValue("GroundParameters","GroundTopHeight","0");
            int out_h = Convert.ToInt32(out_most_barrier_height);
            int path_h = Convert.ToInt32(path_height);
            out_most_barrier_height = Convert.ToString(out_h + path_h);
            string bound_width = ini_file.IniReadValue("GroundParameters", "BoundWidth", "0");
            string bound_height = ini_file.IniReadValue("GroundParameters", "BoundHeight", "0");
            this.OutMostBarrierHeightText.Text = out_most_barrier_height;
            this.PathHeightText.Text = path_height;
            this.InnerBarrierHeightText.Text = inner_barrier_height;
            this.BoundWidthText.Text = bound_width;
            this.BoundHeightText.Text = bound_height;

            //model setting

            //map parameter setting
            string algo_kind = ini_file.IniReadValue("AlgorithmKind", "param", "");

        }
        private void OnGroundHeightMethodChanged(object sender, EventArgs e)
        {
            //
            if(this.CmbGroundHeight.SelectedIndex == 0 || 
                this.CmbGroundHeight.SelectedIndex == 1)
            {
                this.CayonOrSmoothGroupBox.Show();
            }
            else
                this.CayonOrSmoothGroupBox.Hide();

        }
        private void OnWaterChecked(object sender, EventArgs e)
        {
            //
            if (this.ChkHasWater.Checked)
            {
                this.WaterParameterGroupBox.Show();
            }
            else
                this.WaterParameterGroupBox.Hide();
        }
        private void LoadFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(!can_work()) return;

            this.openRMFileDialog.FileOk += OnOpenFile;
            this.openRMFileDialog.InitialDirectory = cfg_artist_resource_dir + "\\map"; 
            this.openRMFileDialog.ShowDialog();
           
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
        private Color RGB565ToColor(string str)
        {
            int x = Convert.ToInt32((str));
            Color c;
            //from 	void CMetaSceneClient::LoadDiffuse( CPkgFile& File, uint32 uRegionId )
			int r = ( ( x & 0xf800 )>> 8 ); 
			int g = ( ( x & 0x07e0 )>> 3 ); 
			int b = ( ( x & 0x001f )<< 3 );
            return Color.FromArgb(r,g,b);
        }
        private string ColorToRGB565(Color c)
        {
            int rgb = c.ToArgb();
            int r = (rgb & 0xff0000)>>16;
            int g = (rgb & 0xff00) >> 8 ;
            int b = (rgb & 0xff);
            int x = ((r << 8) & 0xf800) | ((g << 3) & 0x07e0) | ((b >> 3) & 0x001f);
            return Convert.ToString(x);
        }

        private void PathColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this)== DialogResult.OK)
                this.PathColorBtn.BackColor = this.PickColorDialog.Color;
        }

        private void WaterResFileBrowseFileBtn_Click(object sender, EventArgs e)
        {
            if (this.openERSFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.WaterResFileText.Text = this.openERSFileDialog.FileName;
            }
        }

        private void ResSetFileBrowseBtn_Click(object sender, EventArgs e)
        {
            if (this.openERSFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.ResourceSetText.Text = this.openERSFileDialog.FileName;
                ReloadResourceFile(this.ResourceSetText.Text);
            }
        }

        private void PathBorderColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this)== DialogResult.OK)
                this.PathBorderColorBtn.BackColor = this.PickColorDialog.Color;
        }

        private void BarrierColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this) == DialogResult.OK)
                this.BarrierColorBtn.BackColor = this.PickColorDialog.Color;
        }

        private void SunColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this) == DialogResult.OK)
                this.SunColorBtn.BackColor = this.PickColorDialog.Color;
        }

        private void EnvColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this) == DialogResult.OK)
                this.EnvColorBtn.BackColor = this.PickColorDialog.Color;
        }

        private void FogColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this) == DialogResult.OK)
                this.FogColorBtn.BackColor = this.PickColorDialog.Color;
        }

        private void SaveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //save the file
            SaveRMFile(cur_edt_file);
        }

        private void SaveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(this.saveRMFileAsDialog.ShowDialog(this) == DialogResult.OK)
            {
                string new_file_name = this.saveRMFileAsDialog.FileName;
                SaveRMFile(new_file_name);
            }
        }
        private void SaveRMFile(string file_name)
        {
            //todo



            cur_edt_file = file_name;
        }

        private void PathCfgToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(path_config_dlg == null)
                path_config_dlg = new PathConfigPreprocess();
            if(path_config_dlg.ShowDialog(this) == DialogResult.OK)
            {
                //todo: save the config
            }
        }

        private void UpdateResources_Click(object sender, EventArgs e)
        {
            //todo: reload the resources
        }
        private void ReloadResourceFile(string file_name)
        {
            //
            if (ars_res_dlg == null)
                ars_res_dlg = new ARSResources();
            ars_res_dlg.LoadArs(file_name);
            
            ars_res_dlg.Show();

        }
    }
}