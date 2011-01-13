using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Collections;

namespace RandomMapShell
{
    struct ModelConfig{
        private TreeView tv;
        private string config_name;
        //todo

        public ModelConfig(string c_name,TreeView t)
        {
            config_name = c_name;
            tv = t;
        }
        public TreeView tree_view
        {
            get
            { return tv; }
        }
        public string config
        {
            get 
            { return config_name; }
        }
    }
    public partial class RMapShell : Form
    {
        string cfg_artist_resource_dir;
        string cfg_working_dir;//exe path

        string tex_model_resource;
        string water_tex_resource;
        string cur_edt_file;
        ArrayList model_config_table;
        Dictionary<string,string> template_name_2_config_name;

        PathConfigPreprocess path_config_dlg;
        ARSResources ars_res_dlg;

        public RMapShell()
        {
            InitializeComponent();
        }

        private void RMapShell_Load(object sender, EventArgs e)
        {
            PathTexturetreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            BarrierTexturetreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            MixinTexturetreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);

            PathModeltreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            WallModeltreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            SideModeltreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            CornerModeltreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            LinkWalltreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            BarrierModeltreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);

            PathTexturetreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            BarrierTexturetreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            MixinTexturetreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);

            PathModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            WallModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            SideModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            CornerModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            LinkWalltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            BarrierModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            

            PathTexturetreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            BarrierTexturetreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            MixinTexturetreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);

            PathModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            WallModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            SideModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            CornerModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            LinkWalltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            BarrierModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);

            PathTexturetreeView.Nodes.Add(new TreeNode("道路贴图"));
            PathTexturetreeView.ExpandAll();
            BarrierTexturetreeView.Nodes.Add(new TreeNode("障碍贴图"));
            BarrierTexturetreeView.ExpandAll();
            MixinTexturetreeView.Nodes.Add(new TreeNode("边缘过渡贴图"));
            MixinTexturetreeView.ExpandAll();

            PathModeltreeView.Nodes.Add(new TreeNode("道路装饰物"));
            WallModeltreeView.Nodes.Add(new TreeNode("墙"));
            SideModeltreeView.Nodes.Add(new TreeNode("墙边"));
            CornerModeltreeView.Nodes.Add(new TreeNode("角落"));
            LinkWalltreeView.Nodes.Add(new TreeNode("连接墙"));
            BarrierModeltreeView.Nodes.Add(new TreeNode("障碍区"));

            this.ModelSettingCmb.SelectedIndexChanged += OnModelSettingChanged;
            //default setting
            IniFile cfg = new IniFile("rmapshell.ini");
            cfg_artist_resource_dir = cfg.IniReadValue("path","artist_res","");//String.Empty;
            cfg_working_dir = cfg.IniReadValue("path","work_dir","");//String.Empty;
        }

        delegate string GetByIndexFunc(int i);
        delegate void InsertFunc(string[] str, TreeView tv,GetByIndexFunc f);
        private void OnOpenFile(object sender, EventArgs e)
        {
            string file_name = this.openRMFileDialog.FileName;
            if (!File.Exists(file_name)) return;
            cur_edt_file = file_name;
            IniFile ini_file = new IniFile(cur_edt_file);
            tex_model_resource = ini_file.IniReadValue("resources", "decorators");
            string wall = ini_file.IniReadValue("resources", "wall");
            string plant = ini_file.IniReadValue("resources", "plant");
            string texture = ini_file.IniReadValue("resources", "texture");
            string stone = ini_file.IniReadValue("resources", "stone");
            if (tex_model_resource != wall ||
                tex_model_resource != plant ||
                tex_model_resource != stone ||
                tex_model_resource != texture)
            {
                MessageBox.Show(this, "资源路径不一致，你遇到了老的配置文件，暂时不支持！我很抱歉啊！");
                return;
            }
            water_tex_resource = ini_file.IniReadValue("resources", "water");

            //resource setting
            this.WaterResFileText.Text = water_tex_resource;
            this.ResourceSetText.Text = tex_model_resource;
            this.ReloadResourceFile(cfg_artist_resource_dir + "\\" + tex_model_resource);

            //environment setting
            //简化参数，统一设置，不支持时间变化，要是不嫌参数多的话可以支持一下
            string sun_color = ini_file.IniReadValue("mapinfo", "sceneInfo[0].dwSunColor");
            this.SunColorBtn.BackColor = HexToColor(sun_color);
            this.EnvColorBtn.BackColor = HexToColor(ini_file.IniReadValue("mapinfo", "sceneInfo[0].dwAmbientColor"));
            this.FogColorBtn.BackColor = HexToColor(ini_file.IniReadValue("mapinfo", "sceneInfo[0].dwFogColor"));
            this.FogNearText.Text = ini_file.IniReadValue("mapinfo", "sceneInfo[0].fFogStart");
            this.FogFarText.Text = ini_file.IniReadValue("mapinfo", "sceneInfo[0].fFogEnd");
            this.SceneMusicText.Text = ini_file.IniReadValue("Music", "SceneMusic");

            //water
            this.WaterHeightText.Text = ini_file.IniReadValue("WaterParameters", "height");
            string refl_str = ini_file.IniReadValue("WaterParameters", "reflection", "0");
            if (!refl_str.Equals(""))
                this.WaterReflectionCmb.SelectedIndex = Convert.ToInt32(refl_str);
            bool has_water =
                ini_file.IniReadValue("MethodKind", "WaterKind", "NULL") == "AllWater" ||
                ini_file.IniReadValue("MethodKind", "WaterKind", "NULL") == "default";
            this.ChkHasWater.Checked = has_water;
            if (!has_water)
                this.WaterParameterGroupBox.Hide();
            this.ChkHasWater.CheckedChanged += OnWaterChecked;

            string wave_length = ini_file.IniReadValue("mapinfo", "waveInfo.WavePhysicInfo.nWaveSize", "200");
            string wave_period = ini_file.IniReadValue("mapinfo", "nWaveLife", "1500");
            string wave_density = ini_file.IniReadValue("mapinfo", "waveInfo.WavePhysicInfo.nWavePerGrid", "2");
            string water_transparent_height = ini_file.IniReadValue("WaterParameters", "depth", "0");
            this.WaveDensityText.Text = wave_density;
            this.WaveLengthText.Text = wave_length;
            this.WavePeriodText.Text = wave_period;
            this.WaterTransparentHeightText.Text = water_transparent_height;

            //ground setting
            string texture_kind = ini_file.IniReadValue("MethodKind", "RoadKind");
            if (texture_kind.Equals("DefaultTexture"))//2 
            {
                this.CmbTextureMethod.SelectedIndex = 1;
            }
            else if (texture_kind.Equals("NatureTexture"))
            {
                this.CmbTextureMethod.SelectedIndex = 0;
            }
            //color
            this.PathColorBtn.BackColor = RGB565ToColor(ini_file.IniReadValue("GroundParameters", "CentralColor"));
            this.PathBorderColorBtn.BackColor = RGB565ToColor(ini_file.IniReadValue("GroundParameters", "BorderColor"));
            this.BarrierColorBtn.BackColor = RGB565ToColor(ini_file.IniReadValue("GroundParameters", "BarrierColor"));
            //height
            string height_kind = ini_file.IniReadValue("MethodKind", "GroundKind");
            if (height_kind.Equals("Cayon"))
                this.CmbGroundHeight.SelectedIndex = 0;
            else if (height_kind.Equals("Smooth"))
                this.CmbGroundHeight.SelectedIndex = 1;
            else if (height_kind.Equals("Flat"))
                this.CmbGroundHeight.SelectedIndex = 2;
            if (this.CmbGroundHeight.SelectedIndex == 0 ||
                this.CmbGroundHeight.SelectedIndex == 1)
            {
                this.CayonOrSmoothGroupBox.Show();
            }
            else
                this.CayonOrSmoothGroupBox.Hide();

            this.CmbGroundHeight.SelectedIndexChanged += OnGroundHeightMethodChanged;
            string out_most_barrier_height = ini_file.IniReadValue("GroundParameters", "OutBarrierHeight", "0");
            string path_height = ini_file.IniReadValue("GroundParameters", "GroundBaseHeight", "0");
            string inner_barrier_height = ini_file.IniReadValue("GroundParameters", "GroundTopHeight", "0");
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

            string[] m_tex = ini_file.IniReadValue("GroundParameters", "MixinTextureIDs", "").Split(';');
            string[] b_tex = ini_file.IniReadValue("GroundParameters", "BarrierTextureIDs", "").Split(';');
            string[] p_tex = ini_file.IniReadValue("GroundParameters", "PathTextureIDs", "").Split(';');

            GetByIndexFunc get_tex = delegate(int i) { return this.ars_res_dlg.getTextureNameByIndex(i); };
            GetByIndexFunc get_model = delegate(int i) { return this.ars_res_dlg.getModelNameByIndex(i); };
            InsertFunc foo = delegate(string[] tex_array, TreeView tv, GetByIndexFunc f)
            {
                ArrayList tex_list = new ArrayList();
                foreach (string str in tex_array)
                {
                    if(!str.Equals(""))
                        tex_list.Add(Convert.ToInt32(str));
                }
                foreach (int i in tex_list)
                {
                    string tex_name = f(i);
                    TreeNode tn = tv.Nodes[0];
                    tn.Nodes.Add(new TreeNode(tex_name));
                }
            };
            foo(m_tex, this.MixinTexturetreeView,get_tex);
            foo(b_tex, this.BarrierTexturetreeView,get_tex);
            foo(p_tex, this.PathTexturetreeView,get_tex);
            this.MixinTexturetreeView.KeyUp += TreeViewKeyUp;
            this.BarrierModeltreeView.KeyUp += TreeViewKeyUp;
            this.PathTexturetreeView.KeyUp += TreeViewKeyUp;

            //model setting
            //todo set the index
            string wall_kind = ini_file.IniReadValue("MethodKind", "WallKind", "");//default,single,street
            string model_generate_method = ini_file.IniReadValue("MethodKind","ModelGenerateKind","");//default useplace
            this.ModelSettingCmb.SelectedIndex = 0;
            foreach(ModelConfig mc in model_config_table)
            {
                string[] midx = ini_file.IniReadValue("ModelParameters", mc.config, "").Split(';');
                foo(midx, mc.tree_view, get_model);
            }
            //map parameter setting
            string algo_kind = ini_file.IniReadValue("AlgorithmKind", "param", "");

        }
        private void OnGroundHeightMethodChanged(object sender, EventArgs e)
        {
            //
            if (this.CmbGroundHeight.SelectedIndex == 0 ||
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
            if (!can_work()) return;

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
            Int32 x = Convert.ToInt32(str, 16);
            int r = ((x & 0x00ff0000) >> 16);
            int g = ((x & 0x0000ff00) >> 8);
            int b = ((x & 0x000000ff));
            return Color.FromArgb(r, g, b);
        }
        private Color RGB565ToColor(string str)
        {
            int x = Convert.ToInt32((str));
            Color c;
            //from 	void CMetaSceneClient::LoadDiffuse( CPkgFile& File, uint32 uRegionId )
            int r = ((x & 0xf800) >> 8);
            int g = ((x & 0x07e0) >> 3);
            int b = ((x & 0x001f) << 3);
            return Color.FromArgb(r, g, b);
        }
        private string ColorToRGB565(Color c)
        {
            int rgb = c.ToArgb();
            int r = (rgb & 0xff0000) >> 16;
            int g = (rgb & 0xff00) >> 8;
            int b = (rgb & 0xff);
            int x = ((r << 8) & 0xf800) | ((g << 3) & 0x07e0) | ((b >> 3) & 0x001f);
            return Convert.ToString(x);
        }

        private void PathColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this) == DialogResult.OK)
                this.PathColorBtn.BackColor = this.PickColorDialog.Color;
        }

        private void WaterResFileBrowseFileBtn_Click(object sender, EventArgs e)
        {
            if (!can_work()) return;
            openERSFileDialog.InitialDirectory = cfg_artist_resource_dir + "\\scene\\rmapres";
            if (this.openERSFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.WaterResFileText.Text = this.openERSFileDialog.FileName;
            }
        }

        private void ResSetFileBrowseBtn_Click(object sender, EventArgs e)
        {
            if (!can_work()) return;
            openERSFileDialog.InitialDirectory = cfg_artist_resource_dir + "\\scene\\rmapres";
            if (this.openERSFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.ResourceSetText.Text = this.openERSFileDialog.FileName;
                ReloadResourceFile(cfg_artist_resource_dir + "\\" + this.ResourceSetText.Text);
            }
        }

        private void PathBorderColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this) == DialogResult.OK)
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
            if (!can_work()) return;
            SaveRMFile(cur_edt_file);
        }

        private void SaveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.saveRMFileAsDialog.ShowDialog(this) == DialogResult.OK)
            {
                string new_file_name = this.saveRMFileAsDialog.FileName;
                SaveRMFile(new_file_name);
            }
        }
        private void GetNameString(TreeNode tn,ArrayList str_array)
        {
            if (tn.GetNodeCount(false) == 0) str_array.Add(tn.Text);
            for(int i = 0;i < tn.GetNodeCount(false);++i)
            {
                GetNameString(tn.Nodes[i],str_array);
            }
        }
        private string GetTextureString(TreeView tv)
        {
            //
            string str = "";
            TreeNode tn = tv.Nodes[0];
            ArrayList str_arr = new ArrayList();
            GetNameString(tn, str_arr);
            foreach(string s in str_arr)
            {
                int idx = this.ars_res_dlg.getTextureIndex(s);
                str += Convert.ToString(idx);
                str += ";";
            }
            return str;
        }
        private string GetModelString(TreeView tv)
        {
            string str = "";
            TreeNode tn = tv.Nodes[0];
            ArrayList str_arr = new ArrayList();
            GetNameString(tn, str_arr);
            foreach(string s in str_arr)
            {
                int idx = this.ars_res_dlg.getModelIndex(str);
                str += Convert.ToString(idx);
                str += ";";
            }
            return str;
        }
        private void SaveRMFile(string file_name)
        {
            //todo
            if(cur_edt_file != "")
                System.IO.File.Copy(cur_edt_file, file_name, true);
            cur_edt_file = file_name;
            IniFile ini_file = new IniFile(cur_edt_file);
            string tex_model_resource = this.ResourceSetText.Text;
            string water_resource = this.WaterResFileText.Text;
           
            ini_file.IniWriteValue("resources", "decorators",tex_model_resource);
            ini_file.IniWriteValue("resources", "wall",tex_model_resource);
            ini_file.IniWriteValue("resources", "plant",tex_model_resource);
            ini_file.IniWriteValue("resources", "texture",tex_model_resource);
            ini_file.IniWriteValue("resources", "stone",tex_model_resource);
            ini_file.IniWriteValue("resources", "water", water_resource);

            string sun_color = Convert.ToString(this.SunColorBtn.BackColor.ToArgb(),16);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[0].dwSunColor", sun_color);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[1].dwSunColor", sun_color);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[2].dwSunColor", sun_color);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[3].dwSunColor", sun_color);
            string env_color = Convert.ToString(this.EnvColorBtn.BackColor.ToArgb(), 16);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[0].dwAmbientColor", env_color);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[1].dwAmbientColor", env_color);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[2].dwAmbientColor", env_color);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[3].dwAmbientColor", env_color);
            string fog_color = Convert.ToString(this.FogColorBtn.BackColor.ToArgb(), 16);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[0].dwFogColor", fog_color);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[1].dwFogColor", fog_color);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[2].dwFogColor", fog_color);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[3].dwFogColor", fog_color);

            ini_file.IniWriteValue("mapinfo", "sceneInfo[0].fFogStart", this.FogNearText.Text);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[1].fFogStart", this.FogNearText.Text);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[2].fFogStart", this.FogNearText.Text);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[3].fFogStart", this.FogNearText.Text);

            ini_file.IniWriteValue("mapinfo", "sceneInfo[0].fFogEnd", this.FogFarText.Text);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[1].fFogEnd", this.FogFarText.Text);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[2].fFogEnd", this.FogFarText.Text);
            ini_file.IniWriteValue("mapinfo", "sceneInfo[3].fFogEnd", this.FogFarText.Text);

            ini_file.IniWriteValue("Music", "SceneMusic", this.SceneMusicText.Text);

            ini_file.IniWriteValue("WaterParameters", "height", this.WaterHeightText.Text);
            ini_file.IniWriteValue("WaterParameters", "reflection", Convert.ToString(this.WaterReflectionCmb.SelectedIndex));
            ini_file.IniWriteValue("MethodKind", "WaterKind", this.ChkHasWater.Checked ? "AllWater" : "NULL");

            ini_file.IniWriteValue("mapinfo", "waveInfo.WavePhysicInfo.nWaveSize", this.WaveLengthText.Text);
            ini_file.IniWriteValue("mapinfo", "nWaveLife", this.WavePeriodText.Text);
            ini_file.IniWriteValue("mapinfo", "waveInfo.WavePhysicInfo.nWavePerGrid", this.WaveDensityText.Text);
            ini_file.IniWriteValue("WaterParameters", "depth", this.WaterTransparentHeightText.Text);

            string [] texture_kind ={"NatureTexture","DefaultTexture"};
            ini_file.IniWriteValue("MethodKind", "RoadKind", texture_kind[this.CmbTextureMethod.SelectedIndex]);
            ini_file.IniWriteValue("GroundParameters", "CentralColor",ColorToRGB565(this.PathColorBtn.BackColor));
            ini_file.IniWriteValue("GroundParameters", "BorderColor",ColorToRGB565(this.PathBorderColorBtn.BackColor));
            ini_file.IniWriteValue("GroundParameters","BarrierColor",ColorToRGB565(BarrierColorBtn.BackColor));
            string [] ground_kind = {"Cayon","Smooth","Flat"};
            ini_file.IniWriteValue("MethodKind","GroundKind",ground_kind[CmbGroundHeight.SelectedIndex]);

            int out_barrier_height = Convert.ToInt32(this.OutMostBarrierHeightText.Text);
            int ground_base_height = Convert.ToInt32(PathHeightText.Text);
            int inner_barrier_height = Convert.ToInt32(InnerBarrierHeightText.Text);
            int bound_width = Convert.ToInt32(BoundWidthText.Text);
            int bound_height = Convert.ToInt32(BoundHeightText.Text);
            ini_file.IniWriteValue("GroundParameters","OutBarrierHeight",
                Convert.ToString(Math.Max(0,out_barrier_height-ground_base_height)));
            ini_file.IniWriteValue("GroundParameters","GroundBaseHeight",
                Convert.ToString(Math.Max(0,ground_base_height)));
            ini_file.IniWriteValue("GroundParameters","GroundTopHeight",
                Convert.ToString(Math.Max(0,inner_barrier_height)));
            ini_file.IniWriteValue("GroundParameters","BoundHeight",
                Convert.ToString(Math.Max(0,bound_height)));
            ini_file.IniWriteValue("GroundParameters","BoundWidth",
                Convert.ToString(Math.Max(0,bound_width)));

            //save textures
            ini_file.IniWriteValue("GroundParameters", "MixinTextureIDs", GetTextureString(this.MixinTexturetreeView));
            ini_file.IniWriteValue("GroundParameters", "BarrierTextureIDs", GetTextureString(this.BarrierTexturetreeView));
            ini_file.IniWriteValue("GroundParameters", "PathTextureIDs", GetTextureString(this.PathTexturetreeView));


            foreach(ModelConfig mc in model_config_table)
            {
                ini_file.IniWriteValue("ModelParameters", mc.config, GetModelString(mc.tree_view));
            }
        }

        private void PathCfgToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (path_config_dlg == null)
                path_config_dlg = new PathConfigPreprocess();
            if (path_config_dlg.ShowDialog(this) == DialogResult.OK)
            {
                //todo: save the config

                cfg_artist_resource_dir = path_config_dlg.GetArtistResPath();//String.Empty;
                cfg_working_dir = path_config_dlg.GetWorkDir();//String.Empty;

                if (!can_work()) return;
                //write setting
                IniFile cfg = new IniFile("rmapshell.ini");
                cfg.IniWriteValue("path","artist_res",cfg_artist_resource_dir);
                cfg.IniWriteValue("path","work_dir",cfg_working_dir);
            }
        }

        private void UpdateResources_Click(object sender, EventArgs e)
        {
            //todo: reload the resources
            if (!can_work()) return;
            ReloadResourceFile(cfg_artist_resource_dir + "\\" + this.ResourceSetText.Text);
        }
        private void ReloadResourceFile(string file_name)
        {
            //
            if (ars_res_dlg == null)
                ars_res_dlg = new ARSResources();
            ars_res_dlg.ClearArs();
            ars_res_dlg.LoadArs(file_name);

            ars_res_dlg.Show();

        }


        private void tree_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.Copy;
        }

        private void tree_ItemDrag(object sender, ItemDragEventArgs e)
        {
            DoDragDrop(e.Item, DragDropEffects.Copy);
        }

        private void tree_DragDrop(object sender, DragEventArgs e)
        {
            TreeNode NewNode;

            if (e.Data.GetDataPresent("System.Windows.Forms.TreeNode", false))
            {
                //add to root
                TreeNode DestinationNode = ((TreeView)sender).GetNodeAt(new Point(0, 0));
                //    Point pt = ((TreeView)sender).PointToClient(new Point(e.X, e.Y));
                //    TreeNode DestinationNode = ((TreeView)sender).GetNodeAt(pt);
                NewNode = (TreeNode)e.Data.GetData("System.Windows.Forms.TreeNode");

                if (DestinationNode.TreeView != NewNode.TreeView)
                {
                    DestinationNode.Nodes.Add((TreeNode)NewNode.Clone());
                    DestinationNode.Expand();
                    NewNode.Remove();
                }
            }
        }

        private void AboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show(this, "Σ Π");
        }
        private void TreeViewKeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
        { 
            if(e.KeyCode == Keys.Delete)
            {
                //
                TreeView tv = (TreeView)sender;
                TreeNode tn = tv.SelectedNode;
                if (tn != null && tn != tv.Nodes[0])
                {
                    tn.Remove();
                }
            }
        }

        private void QuestionAndFeedbackToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show(this, "(+ PoPo luozhiyuan)");
        }
        private void OnModelSettingChanged(object sender, EventArgs e)
        {
            //todo
            if (model_config_table == null) model_config_table = new ArrayList();
            if (template_name_2_config_name == null) template_name_2_config_name = new Dictionary<string, string>();
            model_config_table.Clear();//"BarrierModel",this.BarrierModeltreeView
            template_name_2_config_name.Clear();//(障碍物件->BarrierModel)
            if(this.ModelSettingCmb.SelectedIndex == 0)
            {
                //for instance
                model_config_table.Add(new ModelConfig("PathModel",this.PathModeltreeView));
            }
            //set up template_name_2_config_name
            //try to use template to load model
           //todo 
           
        }
    }
}