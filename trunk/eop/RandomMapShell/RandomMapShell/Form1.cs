using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Collections;
using System.Diagnostics;

namespace RandomMapShell
{
    struct ModelIntensityConfig{
        private TextBox box;
        private string config_name;
        private string section_name;
        public ModelIntensityConfig(string c_name,TextBox tb)
        {
            config_name = c_name;
            section_name = "ModelParameters";
            box = tb;
        }
        public ModelIntensityConfig(string c_n,string sn,TextBox tb)
        {
            config_name = c_n;
            section_name = sn;
            box = tb;
        }
        public TextBox text_box{
            get { return box; }
        }
        public string config{
            get { return config_name; }
        }
        public string section{
            get { return section_name; }
        }
    }
    struct ModelConfig{
        private TreeView tv;
        private string config_name;
        private string section_name;
        //todo

        public ModelConfig(string c_name,TreeView t)
        {
            config_name = c_name;
            tv = t;
            section_name = "ModelParameters";
        }
        public ModelConfig(string c_name,string s_name,TreeView t)
        {
            config_name = c_name;
            tv = t;
            section_name = s_name;
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
        public string section
        {
            get { return section_name; }
        }
    }

    public partial class RMapShell : Form
    {
        string cfg_artist_resource_dir;
        string cfg_working_dir;//exe path

        string tex_model_resource;
        string water_tex_resource;
        string cur_edt_file;
        model_config_setting other_model_configs;
        ArrayList model_config_table;
        ArrayList model_intensity_table;
        ArrayList all_model_tree_views;
        Dictionary<string,string> template_name_2_config_name;
        Dictionary<int,model_config_setting> selected_index_2_config_combination;

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
            GrasstreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            FlowertreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            MixModeltreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            DecoratorModeltreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            MiddleModeltreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);
            PathModelWithBarriertreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.tree_ItemDrag);

            PathTexturetreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            BarrierTexturetreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            MixinTexturetreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);

            PathModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            WallModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            SideModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            CornerModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            LinkWalltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            BarrierModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            GrasstreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            FlowertreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            MixModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            DecoratorModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            MiddleModeltreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);
            PathModelWithBarriertreeView.DragEnter +=new System.Windows.Forms.DragEventHandler(this.tree_DragEnter);

            PathTexturetreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            BarrierTexturetreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            MixinTexturetreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);

            PathModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            WallModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            SideModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            CornerModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            LinkWalltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            BarrierModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            GrasstreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            FlowertreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            MixModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            DecoratorModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            MiddleModeltreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);
            PathModelWithBarriertreeView.DragDrop +=  new System.Windows.Forms.DragEventHandler(this.tree_DragDrop);

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
            MixModeltreeView.Nodes.Add(new TreeNode("混合区"));
            GrasstreeView.Nodes.Add(new TreeNode("草"));
            FlowertreeView.Nodes.Add(new TreeNode("花"));

            all_model_tree_views = new ArrayList();
            all_model_tree_views.Add(PathModeltreeView);
            all_model_tree_views.Add(WallModeltreeView);
            all_model_tree_views.Add(SideModeltreeView);
            all_model_tree_views.Add(CornerModeltreeView);
            all_model_tree_views.Add(LinkWalltreeView);
            all_model_tree_views.Add(BarrierModeltreeView);
            all_model_tree_views.Add(MixModeltreeView);
            all_model_tree_views.Add(GrasstreeView);
            all_model_tree_views.Add(FlowertreeView);
            all_model_tree_views.Add(DecoratorModeltreeView);
            all_model_tree_views.Add(MiddleModeltreeView);
            all_model_tree_views.Add(PathModelWithBarriertreeView);
            foreach(TreeView tv in all_model_tree_views)
            {
                tv.KeyUp += TreeViewKeyUp; 
            }

            this.ModelSettingCmb.SelectedIndexChanged += OnModelSettingChanged;
            //default setting
            IniFile cfg = new IniFile("rmapshell.ini");
            cfg_artist_resource_dir = cfg.IniReadValue("path","artist_res","");//String.Empty;
            cfg_working_dir = cfg.IniReadValue("path","work_dir","");//String.Empty;

            //initialize some settings
            selected_index_2_config_combination = new Dictionary<int, model_config_setting>();

        //border_interval_of_ModelParameters;
        //public string interior_interval_of_ModelParameters;
        //public string mapTopic_of_ModelParameters;
        //public string WallKind_of_MethodKind;
        //public string DecorationKind_of_MethodKind;
        //public string ModelGenerateKind_of_MethodKind;
        //public string PlantKind_of_Method_Kind;

        //public string MiddleIntensity_of_ModelParameters;
        //public string InterIntensity_of_ModelParameters;
        //public string OuterIntensity_of_ModelParameters;
        //public string GrassIntensity_of_ModelParameters;
        //public string PathIntensity_of_ModelParameters;
        //public string BorderIntensity_of_ModelParameters;
        //public string FlowerIntensity_of_ModelParameters;
            model_config_setting m0 = new model_config_setting("0","0","0","single","PathCorner","NULL","deletant","0","0","0","0","0","0","0");
           // model_config_setting m2 = new model_config_setting();
            selected_index_2_config_combination.Add(0,m0);
        }
        private int getModelCfgIndexByModelConfig(model_config_setting m)
        {
           if (m.mapTopic_of_ModelParameters.Equals("0") && 
               m.DecorationKind_of_MethodKind.Equals("PathCorner") && 
               m.WallKind_of_MethodKind.Equals("single"))
           {
               return 0;
           }
           else if(m.mapTopic_of_ModelParameters.Equals("1") && 
               m.DecorationKind_of_MethodKind.Equals("PathCorner") && 
               m.WallKind_of_MethodKind.Equals("street") &&
               m.ModelGenerateKind_of_MethodKind.Equals("useplace"))
           {
               return 1;
           }
           else if(m.mapTopic_of_ModelParameters.Equals("1") && 
               m.DecorationKind_of_MethodKind.Equals("PathCorner") && 
               m.WallKind_of_MethodKind.Equals("street") &&
               m.ModelGenerateKind_of_MethodKind.Equals("default"))
           {
               return 2;
           }
           else if(m.mapTopic_of_ModelParameters.Equals("1") && 
               m.DecorationKind_of_MethodKind.Equals("PathCorner") && 
               m.WallKind_of_MethodKind.Equals("default") &&
               m.ModelGenerateKind_of_MethodKind.Equals("default"))
           {
               return 3;
           }
           else
           return 4; //use this as template
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
            string wave_period = ini_file.IniReadValue("mapinfo", "waveInfo.WavePhysicInfo.nWaveLife", "1500");
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
            else this.CmbTextureMethod.SelectedIndex = 0;
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
                    if(!FindNodeInTree(tex_name,tv))
                        tn.Nodes.Add(new TreeNode(tex_name));
                }
            };
            foo(m_tex, this.MixinTexturetreeView,get_tex);
            foo(b_tex, this.BarrierTexturetreeView,get_tex);
            foo(p_tex, this.PathTexturetreeView,get_tex);
            this.MixinTexturetreeView.KeyUp += TreeViewKeyUp;
            this.BarrierModeltreeView.KeyUp += TreeViewKeyUp;
            this.PathTexturetreeView.KeyUp += TreeViewKeyUp;
            this.BarrierTexturetreeView.KeyUp += TreeViewKeyUp;

            //model setting
            //todo set the index
            string wall_kind = ini_file.IniReadValue("MethodKind", "WallKind", "");//default,single,street
            string model_generate_method = ini_file.IniReadValue("MethodKind","ModelGenerateKind","");//default useplace
            other_model_configs = new model_config_setting();
            other_model_configs.Read(ini_file);
            this.ModelSettingCmb.SelectedIndex = getModelCfgIndexByModelConfig(other_model_configs);
            foreach(ModelConfig mc in model_config_table)
            {
                string[] midx = ini_file.IniReadValue(mc.section, mc.config, "").Split(';');
                foo(midx, mc.tree_view, get_model);
            }
            foreach(ModelIntensityConfig mic in model_intensity_table)
            {
                string intensity = ini_file.IniReadValue(mic.section, mic.config, "");
                if(!intensity.Equals(""))
                {
                    mic.text_box.Text = intensity;
                }
            }
            //map parameter setting
            ReadTheMapStructure(ini_file);

        }
        private void OnMapStructureChanged(object sender, EventArgs e)
        {
            string[] algo_param ;
            string cur_algo_param = MapParameterText.Text;
            string[] cur_algo_param_splitted = cur_algo_param.Split(',');
            int unit_size_idx = -1;
            string method_name="";
            if(MapTypeCmb.SelectedIndex == 0)
            {
                unit_size_idx = 13;
                method_name = "dungeon";
            }
            else if(MapTypeCmb.SelectedIndex == 1)
            {
                unit_size_idx = 13;
                method_name = "dungeon2";
            }
            else if(MapTypeCmb.SelectedIndex == 2)
            {
                unit_size_idx = 13;
                method_name = "dungeon4";
            }
            else if(MapTypeCmb.SelectedIndex == 3)
            {
                unit_size_idx = 13;
                method_name = "dungeon3";
            }
            else if(MapTypeCmb.SelectedIndex == 4)
            {
                unit_size_idx = 15;
                method_name = "ravine";
            }
            else if(MapTypeCmb.SelectedIndex == 5)
            {
                unit_size_idx = 12;
                method_name = "allrooms";
            }
            else if(MapTypeCmb.SelectedIndex == 6)
            {
                unit_size_idx = 7;
                method_name = "citystreet";
            }
            algo_param = new string[unit_size_idx + 1];
            algo_param[0] = method_name;
            string w = MapWidthText.Text ;
            string h = MapHeightText.Text ;
            string unit_size = MapUnitSizeText.Text ;
            if(cur_algo_param_splitted[0].Equals(algo_param[0]))
            {
                cur_algo_param_splitted[2] = w;
                cur_algo_param_splitted[3] = h;
                cur_algo_param_splitted[unit_size_idx] = unit_size;
                for(int i = 0;i < cur_algo_param_splitted.Length; ++i)
                {
                    algo_param[i] = cur_algo_param_splitted[i];
                }
            }
            else{
                string[] default_dungeon = "dungeon 1 160 80 36 0 0 0 40 57 40 80 89 6".Split(' ');
                string[] default_dungeon2 = "dungeon2 1 96 96 50 100 0 0 40 60 40 80 100 6".Split(' ');
                string[] default_dungeon3 = "dungeon3 1 160 160 50 100 3 3 60 60 80 80 100 6".Split(' ');
                string[] default_dungeon4 = "dungeon4 133353 80 80 50 100 0 0 40 60 40 80 100 6".Split(' ');
                string[] default_allrooms = "allrooms 1 64 64 25 16 60 34 102 31 10 19 1".Split(' ');
                string[] default_citystreet = "citystreet 1 320 320 24 50 17 8".Split(' ');
                string[] default_ravine = "ravine 11 128 128 60 2 4 5 3 4 3 4 89 89 0 10".Split(' ');
                switch(method_name)
                {
                    case "dungeon":
                        for (int i = 0; i < default_dungeon.Length; ++i )
                            algo_param[i] = default_dungeon[i];
                            break;
                    case "dungeon2": 
                        for (int i = 0; i < default_dungeon2.Length; ++i )
                            algo_param[i] = default_dungeon2[i];
                        break;
                    case "dungeon3": 
                        for (int i = 0; i < default_dungeon3.Length; ++i )
                            algo_param[i] = default_dungeon3[i];
                        break;
                    case "dungeon4": 
                        for (int i = 0; i < default_dungeon4.Length; ++i )
                            algo_param[i] = default_dungeon4[i];
                        break;
                    case "allrooms": 
                        for (int i = 0; i < default_allrooms.Length; ++i )
                            algo_param[i] = default_allrooms[i];
                        break;
                    case "citystreet": 
                        for (int i = 0; i < default_citystreet.Length; ++i )
                            algo_param[i] = default_citystreet[i];
                        break;
                    case "ravine":
                        for (int i = 0; i < default_ravine.Length; ++i )
                            algo_param[i] = default_ravine[i];
                        break;
                }
                algo_param[2] = w;
                algo_param[3] = h;
                algo_param[unit_size_idx] = unit_size;
            }
            string text="";
            foreach(string s in algo_param)
            {
                text += s;
                text += " ";
            }
            MapParameterText.Text = text;
        }
        private void ReadTheMapStructure(IniFile ini_file)
        {
            //todo
            string algo_kind = ini_file.IniReadValue("AlgorithmKind", "param", "");
            string[] algo_parameters = algo_kind.Split(',');
            int w, h, unit_size;
                w = Convert.ToInt32(algo_parameters[2]);
                h = Convert.ToInt32(algo_parameters[3]);
                unit_size = 6;
            if(algo_parameters[0].Equals("dungeon"))
            {
                MapTypeCmb.SelectedIndex = 0;
                unit_size = Convert.ToInt32(algo_parameters[13]);
            }
            else if(algo_parameters[0].Equals("dungeon2"))
            {
                MapTypeCmb.SelectedIndex = 1;
                unit_size = Convert.ToInt32(algo_parameters[13]);
            }
            else if(algo_parameters[0].Equals("dungeon3"))//fly
            {
                MapTypeCmb.SelectedIndex = 3;
                unit_size = Convert.ToInt32(algo_parameters[13]);
            }
            else if(algo_parameters[0].Equals("dungeon4"))
            {
                MapTypeCmb.SelectedIndex = 2;
                unit_size = Convert.ToInt32(algo_parameters[13]);
            }
            else if(algo_parameters[0].Equals("allrooms"))
            {
                MapTypeCmb.SelectedIndex = 5;
                unit_size = Convert.ToInt32(algo_parameters[12]);
            }
            else if(algo_parameters[0].Equals("citystreet"))
            {
                MapTypeCmb.SelectedIndex = 6;
                unit_size = Convert.ToInt32(algo_parameters[7]);
            }
            else if(algo_parameters[0].Equals("ravine"))
            {
                MapTypeCmb.SelectedIndex = 4;
                unit_size = Convert.ToInt32(algo_parameters[15]);
            }
            MapWidthText.Text = Convert.ToString(w);
            MapHeightText.Text = Convert.ToString(h);
            MapUnitSizeText.Text = Convert.ToString(unit_size);
            algo_kind = "";
            foreach(string s in algo_parameters)
            {
                algo_kind += s;
                algo_kind += " ";
            }
            MapParameterText.Text = algo_kind;

            MapWidthText.TextChanged += OnMapStructureChanged;
            MapHeightText.TextChanged += OnMapStructureChanged;
            MapUnitSizeText.TextChanged += OnMapStructureChanged;
            MapTypeCmb.SelectedIndexChanged += OnMapStructureChanged;
        }
        private void WriteMapStructure(IniFile ini_file)
        {
            string text = MapParameterText.Text;
            string result = "";
            for (int i = 0; i < text.Length; ++i )
            {
                if (text[i] == ' ')
                {
                    result += ",";
                }
                else
                    result += text[i];
            }
            ini_file.IniWriteValue("AlgorithmKind", "param", result);
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
            {
                this.PathColorBtn.BackColor = this.PickColorDialog.Color;
                PathColortextBox.Text = ColorToRGB565(this.PickColorDialog.Color);
            }
        }

        private string RemovePathString(string prefix,string path)
        {
            int i = 0;
            Debug.Assert(prefix.Length < path.Length);
            for(;i < prefix.Length; ++i)
            {
                if (prefix[i] == path[i] ||
                    (prefix[i] == '\\' &&
                    path[i] == '/') ||
                    (prefix[i] == '/' &&
                    path[i] == '\\')
                    )
                {
                }
                else break;
            }
            return path.Substring(i);
        }
        private void WaterResFileBrowseFileBtn_Click(object sender, EventArgs e)
        {
            if (!can_work()) return;
            openERSFileDialog.InitialDirectory = cfg_artist_resource_dir + "\\scene\\rmapres";
            if (this.openERSFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.WaterResFileText.Text = RemovePathString(cfg_artist_resource_dir,this.openERSFileDialog.FileName);
            }
        }

        private void ResSetFileBrowseBtn_Click(object sender, EventArgs e)
        {
            if (!can_work()) return;
            openERSFileDialog.InitialDirectory = cfg_artist_resource_dir + "\\scene\\rmapres";
            if (this.openERSFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.ResourceSetText.Text = RemovePathString(cfg_artist_resource_dir,this.openERSFileDialog.FileName);
                ReloadResourceFile(cfg_artist_resource_dir + "\\" + this.ResourceSetText.Text);
            }
        }

        private void PathBorderColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.PathBorderColorBtn.BackColor = this.PickColorDialog.Color;
                this.BorderColortextBox.Text = ColorToRGB565(this.PickColorDialog.Color);
            }
        }

        private void BarrierColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.BarrierColorBtn.BackColor = this.PickColorDialog.Color;
                this.BarrierColortextBox.Text = ColorToRGB565(this.PickColorDialog.Color);
            }
        }

        private void SunColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.SunColorBtn.BackColor = this.PickColorDialog.Color;
                this.SunColortextBox.Text = Convert.ToString(this.SunColorBtn.BackColor.ToArgb(),16);
            }
        }

        private void EnvColorBtn_Click(object sender, EventArgs e)
        {
            if (this.PickColorDialog.ShowDialog(this) == DialogResult.OK)
            {
                this.EnvColorBtn.BackColor = this.PickColorDialog.Color;
                this.EnvColortextBox.Text = Convert.ToString(this.EnvColorBtn.BackColor.ToArgb(),16);
            }
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
                cur_edt_file = new_file_name;
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
                int idx = this.ars_res_dlg.getModelIndex(s);
                str += Convert.ToString(idx);
                str += ";";
            }
            return str;
        }
        private void SaveRMFile(string file_name)
        {
            //todo
            if(cur_edt_file != "" && cur_edt_file != file_name)
                System.IO.File.Copy(cur_edt_file, file_name, true);
            IniFile ini_file = new IniFile(file_name);
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
            ini_file.IniWriteValue("mapinfo", "waveInfo.WavePhysicInfo.nWaveLife", this.WavePeriodText.Text);
            ini_file.IniWriteValue("mapinfo", "waveInfo.WavePhysicInfo.nWavePerGrid", this.WaveDensityText.Text);
            ini_file.IniWriteValue("WaterParameters", "visibleDepth", this.WaterTransparentHeightText.Text);

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
            ini_file.IniWriteValue("GroundParameters","BoundaryHeight",
                Convert.ToString(Math.Max(0,bound_height)));
            ini_file.IniWriteValue("GroundParameters","BoundWidth",
                Convert.ToString(Math.Max(0,bound_width)));

            //save textures
            ini_file.IniWriteValue("GroundParameters", "MixinTextureIDs", GetTextureString(this.MixinTexturetreeView));
            ini_file.IniWriteValue("GroundParameters", "BarrierTextureIDs", GetTextureString(this.BarrierTexturetreeView));
            ini_file.IniWriteValue("GroundParameters", "PathTextureIDs", GetTextureString(this.PathTexturetreeView));


            foreach(ModelConfig mc in model_config_table)
            {
                ini_file.IniWriteValue(mc.section, mc.config, GetModelString(mc.tree_view));
            }
            foreach(ModelIntensityConfig mic in model_intensity_table)
            {
                ini_file.IniWriteValue(mic.section, mic.config, mic.text_box.Text);
            }
            WriteMapStructure(ini_file);
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
        private void LoadByTemplate()
        {
            //todo 
            foreach(KeyValuePair<string,string> p in template_name_2_config_name)
            {
                //try to load by folder's name
                ArrayList all_strings = ars_res_dlg.GetAllNames(p.Key);
                if(all_strings != null)
                {
                    foreach(string s in all_strings)
                    {
                        foreach(ModelConfig mc in model_config_table)
                        {
                            if(mc.config.Equals(p.Value) && !FindNodeInTree(s,mc.tree_view))
                            {
                                TreeNode tn = mc.tree_view.Nodes[0];
                                tn.Nodes.Add(new TreeNode(s));
                            }
                        }
                    }
                }
            }
        }
        private bool FindNodeInTree(string str,TreeView tv)
        {
            foreach(TreeNode tn in tv.Nodes[0].Nodes)
            {
                if (str.Equals(tn.Text))
                    return true;
            }
            return false;
        }
        private void ReloadResourceFile(string file_name)
        {
            //
            if (ars_res_dlg == null)
                ars_res_dlg = new ARSResources();
            ars_res_dlg.ClearArs();
            ars_res_dlg.LoadArs(file_name);

            ars_res_dlg.Show();
            ars_res_dlg.GenerateBarrierInfo(cfg_working_dir, cfg_artist_resource_dir, file_name);

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
            MessageBox.Show(this, "todo");
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
            MessageBox.Show(this, "((+ PoPo) luozhiyuan)");
        }
        private void OnModelSettingChanged(object sender, EventArgs e)
        {
            //todo
            if (model_config_table == null) model_config_table = new ArrayList();
            if (model_intensity_table == null) model_intensity_table = new ArrayList();
            if (template_name_2_config_name == null) template_name_2_config_name = new Dictionary<string, string>();
            model_config_table.Clear();//"BarrierModel",this.BarrierModeltreeView
            template_name_2_config_name.Clear();//(障碍物件->BarrierModel)
            model_intensity_table.Clear();
            HideAllModelIntensityTextBox();
            if (this.ModelSettingCmb.SelectedIndex == 0)
            {
                //for instance
                model_config_table.Add(new ModelConfig("LinkWall", this.LinkWalltreeView));
                model_config_table.Add(new ModelConfig("NormalWall", this.WallModeltreeView));
                model_config_table.Add(new ModelConfig("CornerModel", this.CornerModeltreeView));
                model_config_table.Add(new ModelConfig("SideModel", this.SideModeltreeView));
                model_config_table.Add(new ModelConfig("BarrierModel", BarrierModeltreeView));
                model_config_table.Add(new ModelConfig("MixAreaModel", MixModeltreeView));
                model_config_table.Add(new ModelConfig("PathModel", this.PathModeltreeView));
                template_name_2_config_name.Add("连接墙", "LinkWall");
                template_name_2_config_name.Add("普通墙", "NormalWall");
                template_name_2_config_name.Add("角落物体", "CornerModel");
                template_name_2_config_name.Add("靠边物体", "SideModel");
                template_name_2_config_name.Add("障碍区物体", "BarrierModel");
                template_name_2_config_name.Add("混合区物体", "MixAreaModel");
                template_name_2_config_name.Add("道路区物体", "PathModel");


                PathModeltreeView.Nodes[0].Text = ("道路装饰物");
                WallModeltreeView.Nodes[0].Text = ("墙");
                SideModeltreeView.Nodes[0].Text = ("墙边");
                CornerModeltreeView.Nodes[0].Text = ("角落");
                LinkWalltreeView.Nodes[0].Text = ("连接墙");
                BarrierModeltreeView.Nodes[0].Text = ("障碍区");
                MixModeltreeView.Nodes[0].Text = ("混合区");
                GrasstreeView.Nodes[0].Text = ("草");
                FlowertreeView.Nodes[0].Text = ("花");
            }
            else if(this.ModelSettingCmb.SelectedIndex == 1)
            {
                //
                model_config_table.Add(new ModelConfig("BorderResSet", this.LinkWalltreeView));
                model_config_table.Add(new ModelConfig("wall_res_set", this.WallModeltreeView));
                model_config_table.Add(new ModelConfig("corner_res_set", this.CornerModeltreeView));
                model_config_table.Add(new ModelConfig("decorators_res_set", this.SideModeltreeView));
                model_config_table.Add(new ModelConfig("OuterResSet", BarrierModeltreeView));
                model_config_table.Add(new ModelConfig("interior_res_set", MixModeltreeView));
                model_config_table.Add(new ModelConfig("modelUsed", "PathModel", this.PathModeltreeView));
                //wall_res_set
                //interior_res_set
                //decorators_res_set
                //corner_res_set
                //OuterResSet
                //InterResSet
                //MiddleResSet
                //PathResSet
                //BorderResSet
                template_name_2_config_name.Add("墙", "wall_res_set");
                template_name_2_config_name.Add("内部障碍","interior_res_set");
                template_name_2_config_name.Add("装饰物","decorators_res_set");
                template_name_2_config_name.Add("转角","corner_res_set");
                template_name_2_config_name.Add("外层障碍","OuterResSet");
                template_name_2_config_name.Add("内层随机","InterResSet");
                template_name_2_config_name.Add("中障随机","MiddleResSet");
                template_name_2_config_name.Add("道路随机","PathResSet");
                template_name_2_config_name.Add("边缘随机","BorderResSet");
                template_name_2_config_name.Add("道路带障碍随机","modelUsed");

                PathModeltreeView.Nodes[0].Text = ("道路装饰物(带障碍信息)");
                WallModeltreeView.Nodes[0].Text = ("墙");
                SideModeltreeView.Nodes[0].Text = ("墙边小物件");
                CornerModeltreeView.Nodes[0].Text = ("角落墙（只需一种，自动旋转）");
                LinkWalltreeView.Nodes[0].Text = ("障碍边缘其他物件");
                BarrierModeltreeView.Nodes[0].Text = ("障碍区(外层的)");
                MixModeltreeView.Nodes[0].Text = ("障碍区(被道路围住的))");
                GrasstreeView.Nodes[0].Text = ("草");
                FlowertreeView.Nodes[0].Text = ("花");
                other_model_configs.WallKind_of_MethodKind = "street";
                other_model_configs.DecorationKind_of_MethodKind = "PathCorner";
                other_model_configs.ModelGenerateKind_of_MethodKind = "useplace";
                other_model_configs.PlantKind_of_Method_Kind = "deletant";

                PathModelIntensitytextBox.Show();

                model_intensity_table.Add(new ModelIntensityConfig ("Intensity", "PathModel", PathModelIntensitytextBox));
            }
            else if (this.ModelSettingCmb.SelectedIndex == 2)
            {
                //
                model_config_table.Add(new ModelConfig("BorderResSet", this.LinkWalltreeView));
                //model_config_table.Add(new ModelConfig("wall_res_set", this.WallModeltreeView));
                //model_config_table.Add(new ModelConfig("corner_res_set", this.CornerModeltreeView));
                //model_config_table.Add(new ModelConfig("decorators_res_set", this.SideModeltreeView));
                model_config_table.Add(new ModelConfig("OuterResSet", BarrierModeltreeView));
                model_config_table.Add(new ModelConfig("InterResSet", MixModeltreeView));
                model_config_table.Add(new ModelConfig("modelUsed", "PathModel", this.PathModeltreeView));

                LinkWallIntensitytextBox.Show();
                MixModelIntensitytextBox.Show();
                BarrierModelIntensitytextBox.Show();
                PathModelIntensitytextBox.Show();

                model_intensity_table.Add(new ModelIntensityConfig("InterIntensity", MixModelIntensitytextBox));
                model_intensity_table.Add(new ModelIntensityConfig("OuterIntensity",BarrierModelIntensitytextBox));
                model_intensity_table.Add(new ModelIntensityConfig ("BorderIntensity", LinkWallIntensitytextBox));
                model_intensity_table.Add(new ModelIntensityConfig ("Intensity", "PathModel", PathModelIntensitytextBox));


                template_name_2_config_name.Add("墙", "wall_res_set");
                template_name_2_config_name.Add("内部障碍","interior_res_set");
                template_name_2_config_name.Add("装饰物","decorators_res_set");
                template_name_2_config_name.Add("转角","corner_res_set");
                template_name_2_config_name.Add("外层障碍","OuterResSet");
                template_name_2_config_name.Add("内层随机","InterResSet");
                template_name_2_config_name.Add("中障随机","MiddleResSet");
                template_name_2_config_name.Add("道路随机","PathResSet");
                template_name_2_config_name.Add("边缘随机","BorderResSet");
                template_name_2_config_name.Add("道路带障碍随机","modelUsed");

                PathModeltreeView.Nodes[0].Text = ("道路带障碍随机");
                WallModeltreeView.Nodes[0].Text = ("墙");
                SideModeltreeView.Nodes[0].Text = ("装饰物");
                CornerModeltreeView.Nodes[0].Text = ("转角");
                LinkWalltreeView.Nodes[0].Text = ("边缘随机");
                BarrierModeltreeView.Nodes[0].Text = ("外层障碍");
                MixModeltreeView.Nodes[0].Text = ("内部障碍");

                GrasstreeView.Nodes[0].Text = ("草");
                FlowertreeView.Nodes[0].Text = ("花");
                other_model_configs.WallKind_of_MethodKind = "street";
                other_model_configs.DecorationKind_of_MethodKind = "PathCorner";
                other_model_configs.ModelGenerateKind_of_MethodKind = "default";
                other_model_configs.PlantKind_of_Method_Kind = "deletant";
            }
            else if (this.ModelSettingCmb.SelectedIndex == 3)
            {
                //
                model_config_table.Add(new ModelConfig("BorderResSet", this.LinkWalltreeView));
                model_config_table.Add(new ModelConfig("wall_res_set", this.WallModeltreeView));
                model_config_table.Add(new ModelConfig("corner_res_set", this.CornerModeltreeView));
                model_config_table.Add(new ModelConfig("decorators_res_set", this.SideModeltreeView));
                model_config_table.Add(new ModelConfig("OuterResSet", BarrierModeltreeView));
                model_config_table.Add(new ModelConfig("interior_res_set", MixModeltreeView));
                model_config_table.Add(new ModelConfig("modelUsed", "PathModel", this.PathModeltreeView));
                PathModelIntensitytextBox.Show();
                MixModelIntensitytextBox.Show();
                BarrierModelIntensitytextBox.Show();
                LinkWallIntensitytextBox.Show();

                model_intensity_table.Add(new ModelIntensityConfig("InterIntensity", MixModelIntensitytextBox));
                model_intensity_table.Add(new ModelIntensityConfig("OuterIntensity",BarrierModelIntensitytextBox));
                model_intensity_table.Add(new ModelIntensityConfig ("BorderIntensity", LinkWallIntensitytextBox));
                model_intensity_table.Add(new ModelIntensityConfig ("Intensity", "PathModel", PathModelIntensitytextBox));


                template_name_2_config_name.Add("墙", "wall_res_set");
                template_name_2_config_name.Add("内部障碍","interior_res_set");
                template_name_2_config_name.Add("装饰物","decorators_res_set");
                template_name_2_config_name.Add("转角","corner_res_set");
                template_name_2_config_name.Add("外层障碍","OuterResSet");
                template_name_2_config_name.Add("内层随机","InterResSet");
                template_name_2_config_name.Add("中障随机","MiddleResSet");
                template_name_2_config_name.Add("道路随机","PathResSet");
                template_name_2_config_name.Add("边缘随机","BorderResSet");
                template_name_2_config_name.Add("道路带障碍随机","modelUsed");
                
                PathModeltreeView.Nodes[0].Text = ("道路带障碍随机");
                WallModeltreeView.Nodes[0].Text = ("墙");
                SideModeltreeView.Nodes[0].Text = ("装饰物");
                CornerModeltreeView.Nodes[0].Text = ("转角");
                LinkWalltreeView.Nodes[0].Text = ("边缘随机");
                BarrierModeltreeView.Nodes[0].Text = ("外层障碍");
                MixModeltreeView.Nodes[0].Text = ("内部障碍");
                GrasstreeView.Nodes[0].Text = ("草");
                FlowertreeView.Nodes[0].Text = ("花");
                other_model_configs.WallKind_of_MethodKind = "default";
                other_model_configs.DecorationKind_of_MethodKind = "PathCorner";
                other_model_configs.ModelGenerateKind_of_MethodKind = "useplace";
                other_model_configs.PlantKind_of_Method_Kind = "deletant";
            }
            else if (this.ModelSettingCmb.SelectedIndex == 4)
            {
                model_config_table.Add(new ModelConfig("wall_res_set",this.LinkWalltreeView));
                model_config_table.Add(new ModelConfig("BorderResSet",this.WallModeltreeView));
                model_config_table.Add(new ModelConfig("corner_res_set",this.CornerModeltreeView));
                model_config_table.Add(new ModelConfig("PathResSet",this.SideModeltreeView));
                model_config_table.Add(new ModelConfig("OuterResSet",BarrierModeltreeView));
                model_config_table.Add(new ModelConfig("interior_res_set", MixModeltreeView));
                model_config_table.Add(new ModelConfig("InterResSet", this.PathModeltreeView));
                model_config_table.Add(new ModelConfig("decorators_res_set", this.DecoratorModeltreeView));
                model_config_table.Add(new ModelConfig("MiddleResSet", this.MiddleModeltreeView));
                model_config_table.Add(new ModelConfig("modelUsed","PathModel",this.PathModelWithBarriertreeView));
                this.MiddleIntensitytextBox.Text = other_model_configs.MiddleIntensity_of_ModelParameters;
                this.FlowerIntensitytextBox.Text = other_model_configs.FlowerIntensity_of_ModelParameters;
                this.GrassIntensitytextBox.Text = other_model_configs.GrassIntensity_of_ModelParameters;
                this.WallModelIntensitytextBox.Text = other_model_configs.BorderIntensity_of_ModelParameters;
                this.SideModelIntensitytextBox.Text = other_model_configs.PathIntensity_of_ModelParameters;
                this.BarrierModelIntensitytextBox.Text = other_model_configs.OuterIntensity_of_ModelParameters;

                this.MiddleIntensitytextBox.Show();
                this.FlowerIntensitytextBox.Show();
                this.GrassIntensitytextBox.Show();
                this.WallModelIntensitytextBox.Show();
                this.SideModelIntensitytextBox.Show();
                this.BarrierModelIntensitytextBox.Show();
                this.PathModelIntensitytextBox.Show();
                this.PathModelWithBarrierIntensitytextBox.Show();

                model_intensity_table.Add(new ModelIntensityConfig("MiddleIntensity",MiddleIntensitytextBox));
                model_intensity_table.Add(new ModelIntensityConfig("BorderIntensity",WallModelIntensitytextBox));
                model_intensity_table.Add(new ModelIntensityConfig("InterIntensity", PathModelWithBarrierIntensitytextBox));
                model_intensity_table.Add(new ModelIntensityConfig("OuterIntensity",BarrierModelIntensitytextBox));
                model_intensity_table.Add(new ModelIntensityConfig ("Intensity", "PathModel", PathModelWithBarrierIntensitytextBox));

                
                int idx = 0;
                foreach(TreeView tv in all_model_tree_views)
                {
                    if (tv.GetNodeCount(false) == 0) tv.Nodes.Add(new TreeNode("blank"));
                    tv.Nodes[0].Text = "模型组";
                    tv.Nodes[0].Text += Convert.ToString(idx);
                    idx++;
                }

                //if(other_model_configs.DecorationKind_of_MethodKind.Equals("PathCorner"))
                //{
                //    template_name_2_config_name.Add("靠边物体", "decorators_res_set");
                //}

                //if(other_model_configs.ModelGenerateKind_of_MethodKind.Equals("default"))
                //{
                //    template_name_2_config_name.Add("障碍区物体", "OuterResSet");
                //    template_name_2_config_name.Add("混合区物体", "BorderResSet");
                //    template_name_2_config_name.Add("道路区物体", "PathResSet");
                //}
                //else if(other_model_configs.ModelGenerateKind_of_MethodKind.Equals("useplace"))
                //{
                //    template_name_2_config_name.Add("普通墙", "BorderResSet");
                //    template_name_2_config_name.Add("障碍区物体", "InterResSet");
                //    template_name_2_config_name.Add("混合障碍区物体", "OuterResSet");
                //}

                //if(other_model_configs.WallKind_of_MethodKind.Equals("default"))
                //{
                //     template_name_2_config_name.Add("连接墙", "wall_res_set");
                //    template_name_2_config_name.Add("角落物体", "corner_res_set");
                //}
                //else if(other_model_configs.WallKind_of_MethodKind.Equals("street"))
                //{
                //    template_name_2_config_name.Add("角落物体", "corner_res_set");
                //    template_name_2_config_name.Add("普通墙", "wall_res_set");
                //}

            }
            //set up template_name_2_config_name

            //try to use template to load model
            LoadByTemplate();
        }

        private void PreviewToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Process preview_scene = new Process();
            preview_scene.StartInfo.FileName = cfg_working_dir + "bin\\Release\\";
            preview_scene.StartInfo.FileName += "PreviewRandomScene.exe";
            string rm_dir = System.IO.Path.GetDirectoryName(cur_edt_file);
            string file_name = rm_dir + "\\tmp" + ".rm";
            SaveRMFile(file_name);

            IniFile ini_file = new IniFile(file_name);
            string algorithm_string = ini_file.IniReadValue("AlgorithmKind", "param","");

            string cur_file_name = System.IO.Path.GetFileName(cur_edt_file);
           
            string file_without_postfix = cur_file_name.Substring(0, cur_file_name.Length-3);
            preview_scene.StartInfo.Arguments = "*RANDOM*"+file_without_postfix+"," + algorithm_string + " "+ file_name +" " +"0";
            preview_scene.StartInfo.WorkingDirectory = cfg_working_dir;
            bool bv = (System.IO.File.Exists(preview_scene.StartInfo.FileName));
            preview_scene.Start();
        }
        private void HideAllModelIntensityTextBox()
        {
            PathModelIntensitytextBox.Hide();
            WallModelIntensitytextBox.Hide();
            SideModelIntensitytextBox.Hide();
            MiddleIntensitytextBox.Hide();
            CornerModelIntensitytextBox.Hide();
            BarrierModelIntensitytextBox.Hide();
            LinkWallIntensitytextBox.Hide();
            DecoratorIntensitytextBox.Hide();
            MixModelIntensitytextBox.Hide();
            PathModelWithBarrierIntensitytextBox.Hide();
            GrassIntensitytextBox.Hide();
            FlowerIntensitytextBox.Hide();
        }

        private void SceneFilePreprocessBtn_Click(object sender, EventArgs e)
        {
            //SceneUsage
            //OutputMapFileName
            IniFile ini_file = new IniFile(cur_edt_file);
            string algorithm_string = "allrooms,1,320,320,25,16,60,34,102,31,10,19,1";//ini_file.IniReadValue("AlgorithmKind", "param","");
            string rm_dir = System.IO.Path.GetDirectoryName(cur_edt_file);
            string file_name = rm_dir + "\\tmp" + ".rm";

            string input_file = "map/" + ArtistSceneFileText.Text + ";";
            string out_file =  "map/" + OutPutFileScenePathText.Text ;

            ini_file.IniWriteValue("scene","filename",input_file);
            ini_file.IniWriteValue("scene","SceneUsage","1");//split the scene
            ini_file.IniWriteValue("scene", "OutputMapFileName", out_file);

            //split
            Process preview_scene = new Process();
            preview_scene.StartInfo.FileName = cfg_working_dir + "bin\\Release\\";
            preview_scene.StartInfo.FileName += "PreviewRandomScene.exe";
            SaveRMFile(file_name);

            string cur_file_name = System.IO.Path.GetFileName(cur_edt_file);
            string file_without_postfix = cur_file_name.Substring(0, cur_file_name.Length-3);
            preview_scene.StartInfo.Arguments = "*RANDOM*"+file_without_postfix+"," + algorithm_string + " "+ file_name +" " +"1";
            preview_scene.StartInfo.WorkingDirectory = cfg_working_dir;
            bool bv = (System.IO.File.Exists(preview_scene.StartInfo.FileName));
            preview_scene.Start();
            preview_scene.WaitForExit();
            //////////////////////////////////////////////////////////////////////////
            string inputs=out_file + "_X;";
            inputs += out_file + "_T;";
            inputs += out_file + "_L;";
            inputs += out_file + "_O;";
            inputs += out_file + "_I;";

            inputs += out_file + "_90_T;";
            inputs += out_file + "_90_L;";
            inputs += out_file + "_90_O;";
            inputs += out_file + "_90_I;";

            inputs += out_file + "_180_T;";
            inputs += out_file + "_180_L;";
            inputs += out_file + "_180_O;";

            inputs += out_file + "_270_T;";
            inputs += out_file + "_270_L;";
            inputs += out_file + "_270_O;";
            //copy scene to dest directory
            //string[] all_files = Directory.GetFiles(cfg_artist_resource_dir);
            //foreach(string fnwithp in Directory.GetFiles(cfg_artist_resource_dir))
            //{
            //    string fn = Path.GetFileName(fnwithp);
            //    foreach(string s in inputs.Split(';'))
            //    {
            //        string fstr = Path.GetFileName(s);
            //        if(!s.Equals("") && fn.StartsWith(fstr))
            //        {
            //            File.Move(fn, cfg_artist_resource_dir + "/" +out_file);
            //            break;
            //        }
            //    }
            //}

            ini_file.IniWriteValue("scene","filename",inputs);
            ini_file.IniWriteValue("scene","SceneUsage","0");//split the scene

            ini_file.IniWriteValue("scene", "OutputMapFileName", out_file);
            //generate connection
            Process generate_connection = new Process();
            generate_connection.StartInfo.FileName = cfg_working_dir + "bin\\Release\\";
            generate_connection.StartInfo.FileName += "GenerateConnection.exe";
            SaveRMFile(file_name);

            cur_file_name = System.IO.Path.GetFileName(cur_edt_file);
           
            file_without_postfix = cur_file_name.Substring(0, cur_file_name.Length-3);
            generate_connection.StartInfo.Arguments = "*RANDOM*"+file_without_postfix+"," + algorithm_string + " "+ file_name +" " +"0";
            generate_connection.StartInfo.WorkingDirectory = cfg_working_dir;
             bv = (System.IO.File.Exists(generate_connection.StartInfo.FileName));
            generate_connection.Start();
            generate_connection.WaitForExit();
             
            //set up connections
            string pl="", pu="", pr="", pd="", clr="", cud="";
            foreach(string s in File.ReadAllLines(cfg_artist_resource_dir + "\\connections.txt"))
            {
                if(s.StartsWith("PL="))
                {
                    pl = s.Substring(3);
                }
                if(s.StartsWith("PU="))
                {
                    pu = s.Substring(3);
                }
                if(s.StartsWith("PR="))
                {
                    pr = s.Substring(3);
                }
                if(s.StartsWith("PD="))
                {
                    pd = s.Substring(3);
                }
                if(s.StartsWith("CLR="))
                {
                    clr = s.Substring(4);
                }
                if(s.StartsWith("CUD="))
                {
                    cud = s.Substring(4);
                }
            }
            string clr_processed = "",cud_processed = "";
            int[] o_type =  {3,7,11,14 };
            foreach(string s in clr.Split(';'))
            {
                string[] s2 = new string[2];
                int idx = 0;
                foreach(string s0 in s.Split(','))
                {
                    s2[idx] = s0;
                    idx++; 
                }
                if (s2[0].Equals(""))
                    break;
                int a = Convert.ToInt32(s2[0]);
                int b = Convert.ToInt32(s2[1]);
                bool all_o = false;
                foreach(int k in o_type)
                {
                    if(k == a)
                    {
                        foreach(int kk in o_type)
                        {
                            if(b == kk)
                            {
                                all_o = true;
                            }
                        }
                    }
                }
                if(!all_o)
                {
                    clr_processed += s + ";";
                }
            }

            foreach(string s in cud.Split(';'))
            {
                string[] s2 = new string[2];
                int idx = 0;
                foreach(string s0 in s.Split(','))
                {
                    s2[idx] = s0;
                    idx++; 
                }
                if (s2[0].Equals(""))
                    break;
                int a = Convert.ToInt32(s2[0]);
                int b = Convert.ToInt32(s2[1]);
                bool all_o = false;
                foreach (int k in o_type)
                {
                    if (k == a)
                    {
                        foreach (int kk in o_type)
                        {
                            if (b == kk)
                            {
                                all_o = true;
                            }
                        }
                    }
                }
                if (!all_o)
                {
                    cud_processed += s + ";";
                }
            }
            ini_file.IniWriteValue("scene","filename",inputs);
            ini_file.IniWriteValue("scene","SceneUsage","0");//split the scene
            ini_file.IniWriteValue("scene_shuffle", "PL", pl);
            ini_file.IniWriteValue("scene_shuffle", "PU", pu);
            ini_file.IniWriteValue("scene_shuffle", "PR", pr);
            ini_file.IniWriteValue("scene_shuffle", "PD", pd);
            ini_file.IniWriteValue("scene_shuffle", "CLR", clr_processed);
            ini_file.IniWriteValue("scene_shuffle", "CUD", cud_processed);
            //
            string width = CutAndPasteSceneWidthText.Text;
            string height = CutAndPasteSceneHeightText.Text;
            algorithm_string = "scene_shuffle,1," + width + "," + height + ",";
            ini_file.IniWriteValue("AlgorithmKind", "param", algorithm_string);
        }
    }
}