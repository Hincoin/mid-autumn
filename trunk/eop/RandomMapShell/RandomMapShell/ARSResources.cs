using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.IO;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.Collections;

namespace RandomMapShell
{
    public partial class ARSResources : Form
    {
        string ars_file_name;
        public ARSResources()
        {
            InitializeComponent();
        }

        private void ARSResources_Load(object sender, EventArgs e)
        {
            this.ARSResourcetreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.treeView_ItemDrag);
            this.ARSResourcetreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.treeView_DragEnter);
        }
        public void GenerateBarrierInfo(string working_dir,string artist_res,string file_name)
        {
            //generate_barrier

            string wdir = working_dir ;
            string file_name_no_path = System.IO.Path.GetFileNameWithoutExtension(file_name);
            //todo: call the generate process
            //generate the bounding box info
            Process gen_barrier = new Process();
            gen_barrier.StartInfo.FileName = wdir + "bin\\Release\\";
            gen_barrier.StartInfo.FileName += "MapGenUtility.exe";
            gen_barrier.StartInfo.Arguments = "";
            gen_barrier.StartInfo.Arguments += "generate_barrier" + " " + file_name +" "+  artist_res + "/scene/AllArpLIst.txt" + " " + artist_res + "/map/"+ file_name_no_path+".rmp";
            gen_barrier.StartInfo.WorkingDirectory = wdir + "bin\\Release\\";
            Debug.Assert(System.IO.File.Exists(gen_barrier.StartInfo.FileName));
            gen_barrier.Start();
            gen_barrier.WaitForExit();
            //add to all_rmp.txt
            string all_rmp = artist_res + "/map/all_rmp.txt";
            string[] all_lines = System.IO.File.ReadAllLines(all_rmp, Encoding.GetEncoding("gb2312"));
            foreach(string line in all_lines)
            {
                //
                if (line.Equals(file_name_no_path+".rmp"))
                {
                    return;
                }
            }
           string[] new_lines = new string[all_lines.Length + 1];
           new_lines[all_lines.Length] = file_name_no_path + ".rmp";
            for(int i = 0;i < all_lines.Length ; ++i)
            {
                new_lines[i] = all_lines[i];
            }
            System.IO.File.Delete(all_rmp);
            System.IO.File.WriteAllLines(all_rmp, new_lines, Encoding.GetEncoding("gb2312"));
        }
        public ArrayList GetAllNames(string fold_name)
        {
            //
            ArrayList arr = null;
            foreach (TreeNode tn in ARSResourcetreeView.Nodes[0].Nodes[0].Nodes)
            {
                if (tn.Text.Equals(fold_name))
                {
                    arr = new ArrayList();
                    foreach (TreeNode subtn in tn.Nodes)
                    {
                        arr.Add(subtn.Text);
                    }
                }
            }
            if (arr == null)
                foreach (TreeNode tn in ARSResourcetreeView.Nodes[0].Nodes[1].Nodes)
                {
                    if (tn.Text.Equals(fold_name))
                    {
                        arr = new ArrayList();
                        foreach (TreeNode subtn in tn.Nodes)
                        {
                            arr.Add(subtn.Text);
                        }
                    }
                }
            return arr;
        }
        public void LoadArs(string file_name)
        {
            //
            ars_file_name = file_name;
            string s = System.IO.File.ReadAllText(file_name, Encoding.GetEncoding("gb2312"));

            string new_s = Regex.Replace(s, " (?<var>\\d{1,6})", " P_${var}");
            string new_file_name = file_name + ".tmp";
            System.IO.File.WriteAllText(new_file_name, new_s, Encoding.GetEncoding("gb2312"));

            try
            {
                XmlReader r = new XmlTextReader(new StreamReader(new_file_name, Encoding.GetEncoding("gb2312")));
                XmlDocument dom = new XmlDocument();
                dom.Load(r);
                ARSResourcetreeView.Nodes.Add(new TreeNode(dom.DocumentElement.Name));
                TreeNode tNode = new TreeNode();
                tNode = ARSResourcetreeView.Nodes[0];

                ////XmlNodeReader rdr = new XmlNodeReader(dom);
                ////string str="";
                ////while (rdr.Read())
                ////{
                ////    switch (rdr.NodeType)
                ////    {
                ////        case XmlNodeType.Element:
                ////            str = rdr.Name; break;
                ////        case XmlNodeType.Text:
                ////            if (str.Equals("ResSetGroup"))
                ////            {
                ////                tNode.Nodes.Add(new TreeNode(rdr.Value));
                ////                tNode = tNode.Nodes[0];
                ////            }
                ////            break;
                ////        default:
                ////            break;
                ////    }
                ////}


                // SECTION 3. Populate the TreeView with the DOM nodes.
                AddNode(dom.DocumentElement, tNode);
                ARSResourcetreeView.ExpandAll();
            }
            catch (XmlException xmlEx)
            {
                MessageBox.Show(xmlEx.Message);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        public void ClearArs()
        {
            this.ARSResourcetreeView.Nodes.Clear();
        }
        public void Reload()
        {
            if(ars_file_name != "")
            {
                ClearArs();
                LoadArs(ars_file_name);
            }
        }
        private void AddNode(XmlNode inXmlNode, TreeNode inTreeNode)
        {
            XmlNode xNode;
            TreeNode tNode;
            XmlNodeList nodeList;
            int i;

            // Loop through the XML nodes until the leaf is reached.
            // Add the nodes to the TreeView during the looping process.
            if (inXmlNode.HasChildNodes)
            {
                nodeList = inXmlNode.ChildNodes;
                for (i = 0; i <= nodeList.Count - 1; i++)
                {
                    xNode = inXmlNode.ChildNodes[i];
                    string v = "";
                    if (xNode.Name == "ResSetGroup" ||
                        xNode.Name == "大唐图素集3D" ||
                        xNode.Name == "大唐图素集2D")
                    {
                        if (xNode.Name == "ResSetGroup")
                            v = xNode.Attributes["ResSetGroupName"].Value.ToString();
                        else
                            v = xNode.Name;
                        inTreeNode.Nodes.Add(new TreeNode(v));
                        tNode = inTreeNode.Nodes[i];
                        AddNode(xNode, tNode);
                    }
                    else if (xNode.Name == "ResSetUnit")
                    {
                        if (xNode.Attributes["TextureFileName"] == null)
                            v = xNode.Attributes["P_0Name"].Value.ToString();
                        else
                            v = xNode.Attributes["TextureFileName"].Value.ToString();
                        tNode = new TreeNode(v);
                        inTreeNode.Nodes.Add(tNode);
                    }
                    else
                    {
                        tNode = inTreeNode;
                        AddNode(xNode, tNode);
                    }
                }
            }
            else
            {
                // Here you need to pull the data from the XmlNode based on the
                // type of node, whether attribute values are required, and so forth.
                inTreeNode.Text = (inXmlNode.OuterXml).Trim();
            }
        }
        private void treeView_ItemDrag(object sender,
                  System.Windows.Forms.ItemDragEventArgs e)
        {
            DoDragDrop(e.Item, DragDropEffects.Copy);
        }
        private void treeView_DragEnter(object sender, System.Windows.Forms.DragEventArgs e)
        {
            e.Effect = DragDropEffects.Copy;
        }
        public string getTextureNameByIndex(int idx)
        {
            int index = 0;
            TreeNode root = this.ARSResourcetreeView.Nodes[0];

            TreeNode texNode = root.Nodes[1];
            
            int cnt = texNode.GetNodeCount(false);
            for (int i = 0;i < cnt; ++i)
            {
                TreeNode node = texNode.Nodes[i];
                for(int j = 0;j < node.GetNodeCount(false);++j)
                {
                    if (idx == index)
                        return node.Nodes[j].Text;
                    index++;
                }
            }
            return "";
        }
        public int getTextureIndex(string texture_file_name)
        {
            //
            int index = 0;
            TreeNode root = this.ARSResourcetreeView.Nodes[0];

            TreeNode texNode = root.Nodes[1];
            
            int cnt = texNode.GetNodeCount(false);
            for (int i = 0;i < cnt; ++i)
            {
                TreeNode node = texNode.Nodes[i];
                for(int j = 0;j < node.GetNodeCount(false);++j)
                {
                    if (node.Nodes[j].Text == texture_file_name)
                        return index;
                    index++;
                }
            }
            return index;
        }
        public int getModelIndex(string model_file_name)
        {
            //
            int index = 0;

            TreeNode root = this.ARSResourcetreeView.Nodes[0];

            TreeNode texNode = root.Nodes[0];
            
            int cnt = texNode.GetNodeCount(false);
            for (int i = 0;i < cnt; ++i)
            {
                TreeNode node = texNode.Nodes[i];
                for(int j = 0;j < node.GetNodeCount(false);++j)
                {
                    if (node.Nodes[j].Text == model_file_name)
                        return index;
                    index++;
                }
            }
            return index;
        }
        public string getModelNameByIndex(int idx)
        {
            int index = 0;
            TreeNode root = this.ARSResourcetreeView.Nodes[0];

            TreeNode texNode = root.Nodes[0];
            
            int cnt = texNode.GetNodeCount(false);
            for (int i = 0;i < cnt; ++i)
            {
                TreeNode node = texNode.Nodes[i];
                for(int j = 0;j < node.GetNodeCount(false);++j)
                {
                    if (idx == index)
                        return node.Nodes[j].Text;
                    index++;
                }
            }
            return "";
        }

    }

}