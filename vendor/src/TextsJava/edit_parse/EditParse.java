package edit_parse;

import CP.Ebnf1.Ebnf1_EBNF;
import texts.Shared;
import tree.TreeNode;


public class EditParse {
	
	Shared shared;
	
	TreeNode root=null;
	
	public EditParse() {
		this.shared=new Shared();				
		System.out.println("EditParse Constructor nach Ebnf.compile");
	}
	
	//
	private boolean stop=false;
	
	
	public void startThread(){		
	     ThreadWord t1 = new ThreadWord(shared);	    
	     t1.start();	   
	}
	
	// 
	class  ThreadWord extends Thread {
		   private Shared shared;
		  
		   
		   public ThreadWord(Shared shared) {
		      this.shared = shared;		     
		   }
		   
		   // here to enter editParse CP
		  /* private void word(){
			   StringBuffer word=new StringBuffer();
			   System.out.println("word entry");
			   while(true){
				   try {
					   RichChar ch= shared.getSym();
					   System.out.println("word ch: "+ch.ch);
					   if ((ch.ch!=' ') && (ch.ch != '\r')&&(ch.ch!='\n')&&(ch.ch!='$')) word.append(ch);
					   else {System.out.println(); 
					   		stop=ch.ch=='$';
					   		break;
					   	};
				   
				   }catch (Exception e) {
				   
				   }
			   
			   }
			   System.out.println("word: "+word);
			   System.out.println("word exit");
		   }// word
		   
		   */
		  
		   private void syntaxDrivenEdit (Ebnf1_EBNF ebnf){
			  System.out.println("EditParse.syntaxDrivenEdit vor Ebnf.parse");
			  //
			  while (true){
						  
				  try {
					  root=ebnf.syntaxDrivenParse();
					  if (root!=null)
						  {System.out.println("EditParse.syntaxDrivenEdit Ebnf.parse true");
						  
						 /*				 Node iNode=new Node();						 
						 SyntaxTree.walker(iNode, root,null);
						 */
						 break;}
					  else 
					  	{System.out.println
						 ("EditParse.syntaxDrivenEdit Ebnf.syntaxDrivenParse failed ErrorPosition: "
					  	 +/*texts.Shared*/shared.getMaxPos());
					  	break;}
			  			  
				  } catch (Exception e){
					  
				  }stop=true; 
				  //System.out.println("vor walker");
				  //Node iNode=new Node();
				  //SyntaxTree.walker(iNode, root,null);
				   
			  } 
		   }//syntaxDrivenEdit
	
		   
		   public void run() {
			  //Ebnf_TreeNode rootNode=null;
			  Ebnf1_EBNF ebnf=new Ebnf1_EBNF();
			  if (ebnf.init(this.shared));
			  //stop=true;
			  System.out.println("TestEditParse after Ebnf.Init ThrWord Thread run entry");
			  while(!stop){
				
				  try {
					  System.out.println("TestEditParse ThrWord vor syntaxDrivenEdit()");
					  // hier kommt der Parser rein aus dem ein getSym aufgerufen wird,
					  // das  ueber TestShared mit Zeichen aus dem Editor versorgt wird
					 
					  //char ch= shared.getSym();
					  //word();
					  syntaxDrivenEdit(ebnf);
					  
					  
					  System.out.println("TestEditParse ThrWord nach word");
					  // check events here, close or other, e.g. backtrack.
					  while (!Shared.available()) {/* nothing entered */};
					  
					  sleep(10);					  
				  }
				  catch (Exception e){}
			 
				  
			  } //while 
				  
	   } // run
			  
	}

	

	
}
