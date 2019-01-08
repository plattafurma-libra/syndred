package tree;



import texts.Texts;


public class Node /*implements INode*/{

	static final int ROOT =0;
	static final int ENTER =1;
	static final int EXIT =2;
//	public enum Status {ROOT,ENTER, EXIT}
//  enum leads to compiler crash in gpcp 
//  if imported by *.cp;
	
	public static String resultString;
	private StringBuffer sb = new StringBuffer();
	private int previousOp = ROOT;

	public void clearString() {
		sb = new StringBuffer();
		
	}
	
	public String getString(){
		return sb.toString();
	}
	
	public void reset() {
		previousOp = ROOT;
	}
	
	/*private String getParsedToken(TerminalTreeNode node) {
		String res="";
		char ch;
		Texts text=node.getSharedText();
			
		if(text==null) {
			System.out.println("getParsedToken text null node.name: "+
					node.name);	
			return "NOTEXT";
		}
		for (int i=0;i<(node.getEndPos()-node.getStartPos());i++)
		{
			
			ch= text.getTextCharAtPos(i+node.getStartPos());
			//System.out.println("getParsedToken i: "+i+" res: "+res);
			res=res+ch;
		}
		
		return res;
	}*/
	
	// syndred w/ NOTEXT (-> SDE)
	private String getParsedToken(TerminalTreeNode node) {
		String res = "";
		char ch;
		Texts text = node.getSharedText();
		
		if (text == null) return "NOTEXT";
		
		for (int i = 0; i < (node.getEndPos() - node.getStartPos()); i++) {
			ch = text.getRichChars().get(i + node.getStartPos()).ch;
			res = res + ch;
		}
		
		return res;
	}
	
	// syndred
	public void enterNode(TreeNode node, TreeNode parent) {
		TerminalTreeNode tNode = null;
		
		switch (previousOp) {
			case ROOT:
				sb = new StringBuffer();
				break;
			case EXIT:
				// previous node is a sibling or a superior node
				sb.append(",");
				break;
			case ENTER:
				// previous node is a parent node
				break;
		}

		sb.append("{\"name\":\"" + node.nodeName() + "\",");

		if (parent != null) 
			sb.append("\"parent\":\"" + parent.nodeName() + "\",");
		
		if (node instanceof TerminalTreeNode) {
			tNode = (TerminalTreeNode) node;
			String parsedToken = getParsedToken(tNode);
			sb.append("\"match\":\"'" + parsedToken + "'\",");
		}

		sb.append("\"children\":[");

		previousOp = ENTER;
		resultString = sb.toString();
	}
	
	// SDE
	/*public void enterNode(TreeNode node,
			TreeNode parent) {
		TerminalTreeNode tNode=null;
		System.out.println("in EnterNode");
		
		switch(previousOp){
		case ROOT:
			sb = new StringBuffer();
			break;
		case EXIT:
			// previous node is a sibling or a superior node 
			sb.append("\n,");
			break;
		case ENTER:
			// previous node is a parent node
			break;
		}
			
		sb.append("{" + "\n" + "\"" + "name" + "\"" + ": " + node.nodeName() + "),");
		System.out.println("{" + "\n" + "\"" + "name" + "\"" + ": " + node.nodeName() + "),");
		
		if (parent != null) {
			sb.append("\n" + "\"" + "parent" + "\"" + ": " + parent.nodeName() + "),");
			System.out.println("\n" + "\"" + "parent" + "\"" + ": " + parent.nodeName() + "),");
		}
		
		if (node instanceof TerminalTreeNode) {
			tNode = (TerminalTreeNode) node;
			String parsedToken = getParsedToken(tNode);
			sb.append("\n" + "\"" + "match" + "\"" + ": " + parsedToken + "),");
			System.out.println("\n" + "\"" + "match" + "\"" + ": " + parsedToken + "),");
		}
		
		sb.append("\n" + "\"" + "children" + "\"" + ": [");
		System.out.println("\n" + "\"" + "children" + "\"" + ": [");

		previousOp = ENTER;
	};*/
	
	// SDE
	/*public void exitNode(TreeNode node,
			TreeNode mother) {
		System.out.println("in ExitNode");
		
		sb.append("\n" +"]"+"\n" + "}");
		sb.append(",");
		System.out.println("]"+"\n" + "}");
		
		previousOp = EXIT;
	};*/
	
	// syndred
	public void exitNode(TreeNode node, TreeNode mother) {
		switch (previousOp) {
			case ROOT:
			case EXIT:
				// node had children
				sb.append("]}");
				break;
			case ENTER:
				// node has no children, delete children's field
				sb.delete(sb.length() - 13, sb.length());
				sb.append("}");
				break;
		}

		previousOp = EXIT;
		resultString = sb.toString();
	}

}


