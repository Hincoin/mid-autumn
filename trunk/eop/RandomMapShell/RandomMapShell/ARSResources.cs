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

        }
        public void LoadArs(string file_name)
        {
            //
            try{
                string s = System.IO.File.ReadAllText( file_name ,Encoding.GetEncoding("gb2312"));

                string new_s = Regex.Replace(s, " (?<var>\\d{1,6})",
" P_${var}");
                string new_file_name = file_name + ".tmp";
                System.IO.File.WriteAllText(new_file_name,new_s,Encoding.GetEncoding("gb2312"));
                XmlReader r = new XmlTextReader(new StreamReader(new_file_name, Encoding.GetEncoding("gb2312")));
                XmlDocument dom = new XmlDocument();
                dom.Load(r);
                ARSResourcetreeView.Nodes.Add(new TreeNode(dom.DocumentElement.Name));
                TreeNode tNode = new TreeNode();
                tNode = ARSResourcetreeView.Nodes[0];

                // SECTION 3. Populate the TreeView with the DOM nodes.
                AddNode(dom.DocumentElement, tNode);
                ARSResourcetreeView.ExpandAll();
            }
            catch(XmlException xmlEx)
            {
                MessageBox.Show(xmlEx.Message);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        public void ClearArs()
        {

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
            for(i = 0; i<=nodeList.Count - 1; i++)
            {
               xNode = inXmlNode.ChildNodes[i];
               inTreeNode.Nodes.Add(new TreeNode(xNode.Name));
               tNode = inTreeNode.Nodes[i];
               AddNode(xNode, tNode);
            }
         }
         else
         {
            // Here you need to pull the data from the XmlNode based on the
            // type of node, whether attribute values are required, and so forth.
            inTreeNode.Text = (inXmlNode.OuterXml).Trim();
         }
      }             
    }
}