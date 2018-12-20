package tree;

public class SyntaxTree {

	public static void walker(Node iNode, TreeNode node, TreeNode mother) {
		try {
			// if(Texts.ok)System.out.println("walker ");
			if (node != null) {
				// if(Texts.ok)System.out.println("node != null ");
				// entry
				iNode.enterNode(node, mother);
				// depth first
				if (node instanceof NonTerminalTreeNode) {
					TreeNode child = ((NonTerminalTreeNode) node).getChild();				
					walker(iNode, child, node);
				} 
				// exit
				iNode.exitNode(node, mother);

				// breadth second
				walker(iNode, node.suc, mother);
			}
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("node: " + node.name);
		}

	}

}
