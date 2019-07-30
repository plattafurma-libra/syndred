package texts;

//import CP.Ebnf.Ebnf;
//import tree.Node;
//import tree.SyntaxTree;

public class Shared {

	private static int count=0;	
	private static boolean available=false;
	private static int caretPos=0;
	
	private Texts sharedText;// sharedText.textAsCharArray gets (char by char) the chars edited in swtText of
	// the SWT Text Type in the SWT Gui
	
	
	
	public /*static*/ int maxPosInParse;  /* maxPosition of chars read. is used to indicate position
				of error in input   */ 
	public boolean backTrack;
	// parsePos is position of sign processed in EBNF parser;
	// parsePos is compared to caretPos;
	// if caretPos is less than parsePos, backTrack is set to true and parsePos is reset.
	// if caretPos is greater then parsePos, available is true
	//
	
	//public int parsePos=0;//init

	public Shared(){
		backTrack = false;
		maxPosInParse = /*0*/-1;
		this.sharedText=new Texts();
	}
	
	public Texts getSharedText() {
		return this.sharedText;
	}
	
	public void setGrammar(String grammar) {	
		grammar = grammar.concat(".txt");
		sharedText.setGrammar(grammar);
	}
	
	// ch is put to texts.sharedTexts.textAsCharArray from Editor Gui (s. EditParseGUI where setCharFromSWTText is 
	// called (textAsCharArray is private in Shared; thus, it is only accessible by getters and setters).
	// this might be a similar  interface to draft's json objects and their difference in order to 
	// get new chars edited in draft current block
	
	public void setFromSWTText(String textStr,int caretPosFromText,int len){
		// if(Texts.ok) if(Texts.ok) System.out.println("Shared.charFromSWTText ch "+ch);
		// backtrack for insert
		if (!this.backTrack) {
			if (this.sharedText.getParsePos() > caretPosFromText) {
				//18-01-24
				maxPosInParse=this.sharedText.getParsePos();
				this.backTrack=true;
				// parsePos is reset in EBNF parse at the end of backtrack
				if(this.sharedText.ok) System.out.println("Shared.setFromSWTText backTrack true");
			}
		}
		
		
		//if(Texts.ok) System.out.println("Shared.setFromSWTText parsePos: "+
		//this.sharedText.getParsePos());
		this.sharedText.setTextAsRichCharList(textStr);
		//this.internalCaretPos=caretPos;
		if(this.sharedText.ok) System.out.println("Shared.setFromSWTText textStr: "+textStr+
				" parsePos: "+this.sharedText.getParsePos()+ " caretPosFromText: "+
				caretPosFromText);
		// backTrack is set to false after backtrack in ebnf.parse
		
		if(this.backTrack) Shared.available = true;
		else {			
			Shared.available=caretPosFromText >= this.sharedText.getParsePos();
			caretPos=caretPosFromText;
		}
		
	}
	
	/***********************************************************************************/
	/* Vogt, Schildkamp */
	/* proposal for draft's jason, s. comment above  */
	
	/******neu s.o.
	public void setCharFromJson(RichChar richChar){
		// 
		if(Texts.ok) System.out.println("Shared.charFromSWTText ch "+richChar.ch);
		this.sharedText.setTextCharAtPos(richChar.ch, sharedText.getTextLen());
		this.sharedText.incTextLen();
		this.available=sharedText.getTextLen()>sharedText.getTextPos();	
		// backtrack if '$'; alternativ bereits in JavaJson setzen, wie
		// oben in setCharFromSWTText vorausgesetzt und in
		// EditParseGUI valueChanged implementiert
		//
		if(richChar.ch=='$') this.backTrack=true;
	}
	
	// End of Json interfacing   
	***********************************************************************************/
	public int getMaxPos() {
		return maxPosInParse;		
	}
	
	public void setMaxPos(int maxPos) {
		maxPosInParse=maxPos;
	}
	
	public synchronized RichChar getSym() {
		if (Texts.syntaxDriven)
			return getSym_SDE();
		else
			return getSym_syndred();
	}
	
	// SDE
	private RichChar getSym_SDE(){
		if(this.sharedText.ok) System.out.println("Shared.getSym entry");
		if (caretPos>sharedText.getParsePos()) available=true;
		try {
			if(this.sharedText.ok) System.out.println("Shared.getSym nach try vor while");
			while (!available) Thread.sleep(100);
			if(this.sharedText.ok) System.out.println("Shared.getSym pos "+
			this.sharedText.getParsePos());
		}
		catch (InterruptedException e){
			
		}
		available=false;
		//
		//***************************interface draft
		RichChar ch= new RichChar(this.sharedText.getTextCharAtPos
				(this.sharedText.getParsePos()));
		// 18-11-21 now as param above:
		// ch.ch= this.sharedText.getTextCharAtPos(this.sharedText.getParsePos());
		this.sharedText.incParsePos();
		//***************************end interface draft
		// sharedText.text[sharedText.getTextPos()];
		//this.sharedText.incTextPos();
		if(this.sharedText.ok) System.out.println("getSym rch: "+ch.ch);
		
		count=0;
		
		/*
		Node node=new Node();
		if (Ebnf.root==null)if(Texts.ok) System.out.println("Ebnf.root=null");
		SyntaxTree.walker(node,Ebnf.root,null);
		*/
		
		return ch;
	}
	
	// syndred w/o SyntaxTree (-> SDE)
	private RichChar getSym_syndred() {
		try {
			/* 19-05-16 changed >= instead of == */
			while (sharedText.getParsePos() >= /*==*/  sharedText.getTextLen())
				Thread.sleep(100);
		} catch (InterruptedException e) {
			return new RichChar('\0');
		}

		maxPosInParse = -1;
//		SyntaxTree.walker(new Node(), Ebnf.root, null);
		return sharedText.getRichChar();
	}

	public boolean errorCase(int position) {
		if (Texts.syntaxDriven)
			return errorCase_SDE(position);
		else
			return errorCase_syndred(position);
	}
	
	// SDE
	private boolean errorCase_SDE(int position) {
		if (count==0)
		if(this.sharedText.ok) System.out.println("errorCase caretPos: "+caretPos
		+ " maxPosInParse: "+maxPosInParse);
		count=1;
		/*if (getSym()!=null) { 
				this.sharedText.setParsePos(this.sharedText.getParsePos()-1);
				
		if(Texts.ok) System.out.println("errorCase Exit caretPos: "+caretPos);
		}
		*/	
		return (caretPos>maxPosInParse);
	}
	
	// syndred
	private boolean errorCase_syndred(int position) {
		maxPosInParse = Math.max(position, sharedText.getParsePos());

		try {
			Thread.sleep(100);
		} catch (InterruptedException e) {
		}

		return false;
	}
	
	public static boolean available() {
		return available;
	}
}