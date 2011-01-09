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

namespace RandomMapShell
{
    public partial class ARSResources : Form
    {
        public ARSResources()
        {
            InitializeComponent();
        }

        private void ARSResources_Load(object sender, EventArgs e)
        {

            this.ARSResourcetreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.treeView_ItemDrag);
            this.ARSResourcetreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.treeView_DragEnter);
        }
        public void LoadArs(string file_name)
        {
            //

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
                getTextureIndex("shidi");
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
        public int getModelTextureIndex(string model_file_name)
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
                    if (node.Nodes[j].Text == texture_file_name)
                        return index;
                    index++;
                }
            }
            return index;
        }
    }

}