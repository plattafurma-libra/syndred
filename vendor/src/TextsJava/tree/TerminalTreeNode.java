package tree;

public class TerminalTreeNode extends TreeNode {
	
	int startPos;
	int endPos;
	texts.Texts sharedText;
	

	public int  getStartPos(){
		return this.startPos;	
	}
	

 	public int  getEndPos(){
		return this.endPos;	
	}
 	

 	public texts.Texts getSharedText(){	
		return this.sharedText;
 	}

 	public static TerminalTreeNode TerminalTreeNodeFactory
 	(String name, texts.Texts sharedText, int start,int end){
 	 	TerminalTreeNode node =new TerminalTreeNode();
 		node.suc=null;node.name=name;
 		node.startPos=start;node.endPos=end;
 		node.sharedText=sharedText;
 		return node;
 	}
 
 
	
}

