package tree;


public class NonTerminalTreeNode  extends TreeNode{
	
	public TreeNode child;
	
	
	public static NonTerminalTreeNode NonTerminalTreeNodeFactory(String name) {
	
		NonTerminalTreeNode node=new NonTerminalTreeNode();
		node.child=null; node.suc=null;node.name=name;
		return node;
	}
	
}

