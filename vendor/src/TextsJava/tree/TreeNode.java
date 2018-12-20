package tree;

public abstract class TreeNode {
	
	//public static final int NameLen=32;
	String name;
	public TreeNode suc;//for NonTerminal and for Terminal
	
	public String nodeName() {
		return this.name.replaceAll("NONTERMINAL NODE", "NT");
	}
	
	
	public TreeNode getChild() {
	 if(this instanceof NonTerminalTreeNode) 
			return ((NonTerminalTreeNode)this).child;
	 else return null;
	}
	
	public boolean isTerminalTreeNode() {
		return (this instanceof /*Non*/TerminalTreeNode); 
	}
	
}

