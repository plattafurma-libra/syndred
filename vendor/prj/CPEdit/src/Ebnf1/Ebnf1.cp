MODULE Ebnf1;



(* TO DO for terminal nodes which are matched in Regex:
Font Description: 
1. Size:(e.g., 12 point vs. 16 point), 
2. Style (e.g., plain vs. italic), 
3. Typeface (e.g., Times vs. Helvetica),
4. Weight (e.g., bold vs. normal),
5. Color (e.g., red).
*)

(* code mainly from Niklaus Wirth Grundlagen und Techniken des Compilerbaus, from English
version Compiler Construction, too (s. http://www.ethoberon.ethz.ch/WirthPubl/CBEAll.pdf) and for more implementation
details from Wirth Compilerbau Stuttgart Teubner 1986 (for Modula 2).
Changes by JR;
Parser is rewritten to be completely recursive for to establish unlimited backtracking in parse.
*)


(* Wirth example: ebnf defined in ebnf
syntax        =     {production}.           
production    =     identifier "=" expression "." . 
expression    =     term {"|" term}. 
term          =     factor {factor}.              
factor        =     identifier | string | "(" expression ")"
 | "[" expression "]" | "{" expression "}". 
By application of the given translation rules and subsequent simplification the following parser 
results. It is formulated as an Oberon module: 
*)


 
IMPORT RTS,
TextsCP, texts, tree,
 
FontsFont,
RegexApi, 
RegexMatching,
RegexParser,
(*RegexReplace,*)
CPmain; 

CONST IdLen = 32; 
    ident = 0; literal = 2; lparen = 3; lbrak = 4; lbrace = 5; bar = 6; eql = 7; 
    rparen = 8; rbrak = 9; rbrace = 10; period = 11; other = 12; 
    
TYPE Identifier = ARRAY IdLen OF CHAR; 

     Symbol=POINTER TO EXTENSIBLE RECORD alt,next:Symbol 
     END;
     
     Terminal=POINTER TO RECORD(Symbol) sym:INTEGER;
     			name:ARRAY IdLen OF CHAR; 
     	     	reg:RegexApi.Regex;
     	     	
     END;
     
     
     
     Nonterminal = POINTER TO NTSDesc;
	 NTSDesc = RECORD (Symbol) ptrToHeaderList: Header END;
	 Header = POINTER TO HDesc;
	 HDesc = RECORD sym: Symbol; entry: Symbol; suc:Header; name: ARRAY IdLen OF CHAR END;
	 
	 StackPointer = POINTER TO RECORD
		ptr:Element;
	 END;


	 Element = POINTER TO RECORD
		symbol:Symbol;
		suc:Element;
		treeNode:tree.TreeNode;
	 END;
	 
	 (* wrapper for Symbols p, q,r,s which might be used in ebnf as substitute for call by name;
     to be used for reemplimation in java 
     *)     
     SymbolsWrapper =POINTER TO RECORD p,q,r,s:Symbol;
     END;
 
 	 EBNF* = POINTER TO RECORD
 	 			list,sentinel,h:Header;
				q,r,s: Symbol;
				startsymbol:Nonterminal 
				(* startsymbol for parse if called from editor, is exported to 
				call of parse procedure*);

				ch: CHAR; 	 
    			sym:      INTEGER;       
    			lastpos:  INTEGER;  
    			count:INTEGER;
    			id:       Identifier;       
    			R:        TextsCP.Reader;       
    			W:        TextsCP.Writer;       
    			EbnfReader:texts.Texts;
    			iNode:tree.Node;
   
    			shared:texts.Shared;
    
    			parseStack:StackPointer;
        
    			root*:tree.TreeNode; 
 	 		 END (*type ebnf *);
 	 		 
VAR	 ebnf:EBNF;
(******************************	      
VAR list,sentinel,h:Header;
	q,r,s: Symbol;
	startsymbol:Nonterminal (* startsymbol for parse if called from editor, is exported to 
	call of parse procedure*);

	ch: CHAR; 	 
    sym:      INTEGER;       
    lastpos:  INTEGER;  
    count:INTEGER;
    id:       Identifier;       
    R:        TextsCP.Reader;       
    W:        TextsCP.Writer;       
    EbnfReader:texts.Texts;
    iNode:tree.Node;
   
    shared:texts.Shared;
    
    parseStack:StackPointer;
        
    root:tree.TreeNode; 
  *****************************************************) 
   
    
PROCEDURE getNodeName(grammarNode:Symbol;VAR name:ARRAY OF CHAR);

BEGIN
	IF grammarNode=NIL THEN name:="node = Nil"
	ELSIF grammarNode IS Terminal THEN
			name:= "TERMINAL NODE "+grammarNode(Terminal).name$;
	ELSIF grammarNode IS Nonterminal THEN
		name:= "NONTERMINAL NODE "+grammarNode(Nonterminal).ptrToHeaderList.name$;
	ELSE
		name:= "";
	END;
END getNodeName;
  
PROCEDURE (st:StackPointer) Factory(node:Symbol):Element,NEW;
VAR element:Element;
BEGIN
	NEW(element);element.symbol:=node;
	element.suc:=NIL; element.treeNode:=NIL;
	RETURN element;
END Factory;


PROCEDURE (st:StackPointer) push(new:Element),NEW;
VAR old:Element;nodeName:ARRAY IdLen OF CHAR;
BEGIN
	getNodeName(new.symbol,nodeName);
	TextsCP.WriteString(" push: "+nodeName+" ");
	old:=st.ptr;
	new.suc:=old;
	st.ptr:=new;
END push;


PROCEDURE (st:StackPointer) pop():Element,NEW;
VAR element:Element;nodeName:ARRAY IdLen OF CHAR;
BEGIN
	element:=st.ptr;
	IF element = NIL THEN 
		TextsCP.WriteString(" pop Nil");
		TextsCP.WriteLn;
		RETURN NIL 
	ELSE
		st.ptr:=element.suc;
		element.suc:=NIL;
		getNodeName(element.symbol,nodeName);
		TextsCP.WriteString(" pop: "+nodeName);
		TextsCP.WriteLn;
		getNodeName(element.symbol.next,nodeName);
		TextsCP.WriteString(" next: "+nodeName);
		TextsCP.WriteLn;
		RETURN element;
	END;
END pop;
	


PROCEDURE (ebnf:EBNF)error(n: INTEGER),NEW; 
              VAR pos: INTEGER;       
BEGIN pos := TextsCP.Pos(ebnf.R); 
	TextsCP.WriteString("error nr: ");TextsCP.WriteInt(n,2);TextsCP.WriteLn;
    IF pos > ebnf.lastpos+4 THEN  (*avoid spurious error messages*) 
        TextsCP.WriteString("  pos"); TextsCP.WriteInt(pos, 6); 
        TextsCP.WriteString("  err"); TextsCP.WriteInt(n, 4); 
        ebnf.lastpos := pos; 
        TextsCP.WriteString("sym"); TextsCP.WriteInt(ebnf.sym, 4); 
        TextsCP.WriteLn();   (* TextsCP.Append(Oberon.Log,W.buf)   *) 
    END;
    RTS.Throw(" error");       
END error;  

PROCEDURE (ebnf:EBNF)skipBlank,NEW;

BEGIN
	WHILE ~ebnf.EbnfReader.ebnfEot & (ebnf.ch <= " ") DO ebnf.ch:=
		ebnf.EbnfReader.readEbnfChar()
	(*TextsCP.Read(R,ch)*);END;
END skipBlank;  

PROCEDURE (ebnf:EBNF)ReadString(VAR Str:ARRAY OF CHAR),NEW;
VAR i:INTEGER;
BEGIN
	ebnf.skipBlank;
	i:=0;
	WHILE(CAP(ebnf.ch) >= "A") & (CAP(ebnf.ch) <= "Z") DO
		Str[i] :=ebnf.ch;
		INC(i);
		ebnf.ch:=ebnf.EbnfReader.readEbnfChar()(*TextsCP.Read(R,ch)*);
	END;
	Str[i]:=0X;
	ebnf.skipBlank;	
	TextsCP.WriteString(Str);
	TextsCP.WriteLn();
END ReadString;

PROCEDURE (ebnf:EBNF)ReadNumberString(VAR Str:ARRAY OF CHAR),NEW;
VAR i:INTEGER;
BEGIN
	ebnf.skipBlank;
	i:=0;
	WHILE(CAP(ebnf.ch) >= "0") & (CAP(ebnf.ch) <= "9") DO
		Str[i] :=ebnf.ch;
		INC(i);
		ebnf.ch:=ebnf.EbnfReader.readEbnfChar()(*TextsCP.Read(R,ch)*);
	END;
	Str[i]:=0X;
	ebnf.skipBlank;
	TextsCP.WriteString(Str);
	TextsCP.WriteLn();	
END ReadNumberString;

PROCEDURE (ebnf:EBNF)ReadValue(VAR Ptr: POINTER TO ARRAY OF CHAR),NEW;

VAR value:ARRAY IdLen OF CHAR;

BEGIN
	ebnf.ReadString(value);
	RegexApi.ArrayToPointer(value,Ptr);
END ReadValue;

PROCEDURE (ebnf:EBNF)ReadNumberValue(VAR Ptr: POINTER TO ARRAY OF CHAR),NEW;

VAR value:ARRAY IdLen OF CHAR;

BEGIN
	ebnf.ReadNumberString(value);
	RegexApi.ArrayToPointer(value,Ptr);
END ReadNumberValue;

PROCEDURE (ebnf:EBNF)FontDescription():FontsFont.FontDesc,NEW;

VAR attribute, value : ARRAY IdLen OF CHAR;
	fontDesc:FontsFont.FontDesc;
BEGIN
	TextsCP.WriteString("FONTDESCRIPTION  ");
	TextsCP.WriteLn();
	(* next ch after <  *)
	ebnf.ch:=ebnf.EbnfReader.readEbnfChar()(*TextsCP.Read(R,ch)*);
	NEW(fontDesc);
	fontDesc.size:=NIL;fontDesc.style:=NIL; fontDesc.typeface:=NIL;
	fontDesc.weight:=NIL;fontDesc.color:=NIL; 
	WHILE ebnf.ch # '>' DO
		ebnf.ReadString(attribute);
		IF ebnf.ch=':' THEN ebnf.ch:=ebnf.EbnfReader.readEbnfChar()
		(*TextsCP.Read(R,ch)*)
		ELSE ebnf.error(10);
		END;           				
		IF attribute = "Size" THEN 
			ebnf.ReadNumberValue(fontDesc.size)
		ELSIF attribute = "Style" THEN
			ebnf.ReadValue(fontDesc.style);
		ELSIF attribute = "Typeface" THEN
			ebnf.ReadValue(fontDesc.typeface);
		ELSIF attribute = "Weight" THEN
			ebnf.ReadValue(fontDesc.weight);
		ELSIF attribute = "Color" THEN
			ebnf.ReadValue(fontDesc.color);
		ELSE ebnf.error (12);
		END;
		
		IF ebnf.ch= ';' THEN ebnf.ch:=ebnf.EbnfReader.readEbnfChar()
				(*TextsCP.Read(R,ch)*);
				ebnf.skipBlank;
        END;   				
   	END (*while*);  
   	ebnf.ch:=ebnf.EbnfReader.readEbnfChar()(*TextsCP.Read(R,ch)*);
    RETURN fontDesc;      		
END FontDescription; 

PROCEDURE (ebnf:EBNF)GetSym,NEW; 
VAR i:INTEGER;       
BEGIN 
	TextsCP.WriteStringLn("GetSym Entry ch ");
	TextsCP.Write(ebnf.ch);
	IF ebnf.EbnfReader.ebnfEot THEN 
		TextsCP.WriteStringLn("GetSym ebnfEot true");
	END;
	WHILE ~ebnf.EbnfReader.ebnfEot & (ebnf.ch <= " ") DO 
		IF ebnf.EbnfReader.ebnfEot THEN 
			TextsCP.WriteStringLn("GetSym While ebnfEot true");
		END;
		TextsCP.WriteStringLn("GetSym vor While read");
		ebnf.ch:=ebnf.EbnfReader.readEbnfChar();
		TextsCP.WriteStringLn("GetSym nach While read");
		
		
	END ;   (*skip blanks*) 
	TextsCP.WriteStringLn("GetSym vor CASE ch");
    CASE ebnf.ch OF       
   			"A" .. "Z", "a" .. "z": ebnf.sym := ident; i := 0; 
			REPEAT ebnf.id[i] := ebnf.ch; INC(i); 
				ebnf.ch:= ebnf.EbnfReader.readEbnfChar() 
			UNTIL (CAP(ebnf.ch) < "A") OR (CAP(ebnf.ch) > "Z"); 
            ebnf.id[i]:=0X       
			|22X:  (*quote*) 
				ebnf.ch:=ebnf.EbnfReader.readEbnfChar(); 
					ebnf.sym := literal; i := 0; 
				(* wirth----------------------------------- 
				WHILE (ch # 22X) & (ch > " ") DO 
                    ebnf.id[i]:= ch;
                    INC(i);
                    ch:=EbnfReader.readEbnfChar()       
   				END ; 
				IF ch <= " " THEN error(1) END ; 
				------------------------------------------*)
				(* JR, regex *)
				LOOP
					IF ebnf.ch=22X THEN
						IF i=0 (*empty terminal string*) THEN EXIT
						ELSIF ebnf.id[i] # '\'  (* quote is NOT escaped *) THEN EXIT
						ELSIF (i >0) & (ebnf.id[i-1] = '\') (* '\' is escaped by '\', 
						i.e. termination by '"'*)
								THEN EXIT
						END;
					END;
				    ebnf.id[i]:= ebnf.ch;
                    INC(i);
                    IF i > IdLen THEN ebnf.error(1);
                    END;
                    
                    ebnf.ch:=ebnf.EbnfReader.readEbnfChar()       
   				END ; 
				(* Wirth IF ch <= " " THEN error(1) END ;	*)
				ebnf.id[i] := 0X; 
				
				ebnf.ch:=ebnf.EbnfReader.readEbnfChar(); 
				
			|  "=" : ebnf.sym := eql; ebnf.ch:=ebnf.EbnfReader.readEbnfChar()
			|  "(" : ebnf.sym := lparen; ebnf.ch:=ebnf.EbnfReader.readEbnfChar() 
			|  ")" : ebnf.sym := rparen; ebnf.ch:=ebnf.EbnfReader.readEbnfChar()
			|  "[" : ebnf.sym := lbrak; ebnf.ch:=ebnf.EbnfReader.readEbnfChar() 
			|  "]" : ebnf.sym := rbrak; ebnf.ch:=ebnf.EbnfReader.readEbnfChar()
			| "{" : ebnf.sym := lbrace; ebnf.ch:=ebnf.EbnfReader.readEbnfChar()
			|  "}" : ebnf.sym := rbrace;ebnf.ch:=ebnf.EbnfReader.readEbnfChar() 
			| "|" : ebnf.sym := bar; ebnf.ch:=ebnf.EbnfReader.readEbnfChar()
			|  "." : ebnf.sym := period; ebnf.ch:=ebnf.EbnfReader.readEbnfChar() 
			ELSE ebnf.sym := other; 
				(* if entered by jr; otherwise eof error*)
				IF ebnf.EbnfReader.ebnfEot THEN 
					ebnf.ch:=' ' 
				ELSE ebnf.ch:=ebnf.EbnfReader.readEbnfChar();END; 
       		END;
       		TextsCP.WriteStringLn("GetSym Exit");       
END GetSym; 

 
PROCEDURE (ebnf:EBNF)find(str : ARRAY OF CHAR; VAR h:Header),NEW;
VAR h1:Header;
BEGIN
	h1:=ebnf.list;
	ebnf.sentinel.name:=str$;
	WHILE h1.name#str DO h1:=h1.suc;END;
	IF h1 = ebnf.sentinel THEN (*insert*)
		NEW(ebnf.sentinel);
		h1.suc := ebnf.sentinel;
		h1.entry:=NIL;
	END;	
	h:=h1;
END find;

PROCEDURE (ebnf:EBNF)link(p,q:Symbol),NEW;
VAR t:Symbol;

BEGIN (* insert q in places indicated by linked chain p *)
	WHILE p # NIL DO
		t := p; p:=t.next; t.next:=q;
	END;
END link;
   

PROCEDURE (ebnf:EBNF)expression(VAR p,q,r,s:Symbol),NEW;   
VAR q1, s1:Symbol;

    
    PROCEDURE (*ebnf:EBNF*)term(VAR p,q,r,s:Symbol)(*,NEW *);  
    VAR p1,q1,r1,s1:Symbol;     

       PROCEDURE (*ebnf:EBNF*)factor(VAR p,q,r,s:Symbol)(*,NEW*);    
       VAR a:Symbol;identifiernonterminal:Nonterminal;literalterminal:Terminal; h:Header;
       (*regexStr:RTS.NativeString;*)
       BEGIN h:=NIL;a:=NIL;identifiernonterminal:=NIL;literalterminal:=NIL;        		
            IF ebnf.sym = ident (*nonterminal*) THEN
            	NEW(identifiernonterminal);
            	ebnf.find(ebnf.id$,h);
            	(* name of nonterminal symbol may be accessed via h.name);*)
            	identifiernonterminal.ptrToHeaderList:=h;
            	a:=identifiernonterminal;a.alt:=NIL;a.next:=NIL;
            	
            	(*record(T0, id, 1);*)  
            	p:=a;q:=a;r:=a;s:=a;           
            	ebnf.GetSym 
            ELSIF ebnf.sym = literal (*terminal*) THEN 
            	NEW(literalterminal);literalterminal.sym:=ebnf.sym;
            	(*RegexReplace.replaceInRegex(id);*)
            	TextsCP.WriteStringLn
            	(" EBNF.factor vor RegexApi.CreateRegex id:"+ebnf.id$);
            	literalterminal.name:=ebnf.id$; 
            	literalterminal.reg:=RegexApi.CreateRegex(ebnf.id$);
            	(*regexStr:=MKSTR(id$); *)
            	TextsCP.WriteStringLn
            	(" nach CreateRegex id:"+ebnf.id$);
            	a:=literalterminal;a.alt:=NIL;a.next:=NIL;
            	(*record(T1, id, 0);*) 
            	
            	p:=a;q:=a;r:=a;s:=a; 
            	TextsCP.Write(ebnf.ch);            	
            	ebnf.skipBlank();
            	(* fontdescription*)            	
            	IF ebnf.ch = '<' THEN
            		literalterminal.reg.regex.Font:=ebnf.FontDescription();
            	END;		
            	ebnf.GetSym;            	
            	
            
            ELSIF ebnf.sym = lparen THEN 
         		ebnf.GetSym; 
         		ebnf.expression(p,q,r,s); 
                IF ebnf.sym = rparen THEN ebnf.GetSym ELSE ebnf.error(2) END 
            ELSIF ebnf.sym = lbrak THEN 
         		ebnf.GetSym; ebnf.expression(p,q,r,s); 
         		
         		NEW(literalterminal);literalterminal.sym:=ebnf.sym;
            	literalterminal.name:=""; 
            	literalterminal.reg:=NIL;
            	a:=literalterminal;a.alt:=NIL;a.next:=NIL;
            	q.alt:=a;s.next:=a;q:=a;s:=a;
               	IF ebnf.sym = rbrak THEN ebnf.GetSym ELSE ebnf.error(3) END 
            ELSIF ebnf.sym = lbrace THEN 
            	
         		ebnf.GetSym; ebnf.expression(p,q,r,s); 
         		ebnf.link(r,p);
         		TextsCP.WriteStringLn("lbrace nach expression");
         		TextsCP.WriteInt(ebnf.sym,2);
         		TextsCP.WriteLn;
         		
         		NEW(literalterminal);literalterminal.sym:=ebnf.sym;
         		literalterminal.reg:=NIL;
            	literalterminal.name:=""; 
            	a:=literalterminal;a.alt:=NIL;a.next:=NIL;
         		q.alt:=a;q:=a;r:=a;s:=a;
               	IF ebnf.sym = rbrace THEN ebnf.GetSym;               		
               	ELSE ebnf.error(4) END 
            ELSE    ebnf.error(5)    
        	END;        	
        END factor;    
 
    
     BEGIN (*term*) 
      	p1:=NIL;q1:=NIL;r1:=NIL;s1:=NIL;     	
     	factor(p,q,r,s);           
        WHILE ebnf.sym < bar DO 
        	factor(p1,q1,r1,s1);
        	ebnf.link(r,p1);
        	r:=r1;
        	s:=s1; 
        END;            
       
     END term; 
           
    BEGIN (*expression*)  
    	q1:=NIL;s1:=NIL;  
    	
    	term(p,q,r,s);    
    	(* alternatives by '|'*)  
    	WHILE ebnf.sym = bar DO ebnf.GetSym; term(q.alt,q1,s.next,s1);q:=q1;s:=s1; 
    	END;    
    	
    END expression;
 

           
PROCEDURE (ebnf:EBNF)production,NEW;       
BEGIN (*sym = ident*) 
	
	ebnf.find(ebnf.id$,ebnf.h);
	TextsCP.WriteStringLn("production id: "+ebnf.id$);
	
	ebnf.GetSym; 
	
    IF ebnf.sym = eql THEN ebnf.GetSym ELSE ebnf.error(7) END; 
    ebnf.expression(ebnf.h.entry,ebnf.q,ebnf.r,ebnf.s); 
    ebnf.link(ebnf.r,NIL);   
    IF ebnf.sym = period THEN 
    	TextsCP.WriteStringLn("production vor GetSym");
    	IF ebnf.EbnfReader.ebnfEot THEN
    		TextsCP.WriteStringLn("ebnfEot true");
    	END;
    	ebnf.GetSym;
    	TextsCP.WriteStringLn("production nach GetSym");
    ELSE ebnf.error(8) 
    END;
     
END production;


      
PROCEDURE (ebnf:EBNF)syntax,NEW;       
BEGIN    	
	TextsCP.WriteStringLn("syntax start");
    WHILE ebnf.sym = ident DO ebnf.production;
    	TextsCP.WriteStringLn("syntax nach production");
    	IF ebnf.EbnfReader.ebnfEot THEN
    		TextsCP.WriteStringLn("End of file");
    	ELSE TextsCP.WriteStringLn("not end of file");
    	END;
    END;  
    TextsCP.WriteStringLn("syntax end"); 
END syntax;  

(* checks whether there is a nonterminalwhich does not lead to a terminal*)
PROCEDURE (ebnf:EBNF)checkSyntax():BOOLEAN,NEW;
VAR h:Header;error:BOOLEAN;(*i:INTEGER;*)
BEGIN
	TextsCP.WriteLn();
	h:=ebnf.list;error:=FALSE;
	WHILE h # ebnf.sentinel DO	
		IF h.entry=NIL THEN 
			error:=TRUE;
			TextsCP.WriteString("undefined Symbol "+h.name);TextsCP.WriteLn();
			RTS.Throw(" error"); 
		ELSE TextsCP.WriteString("Symbol "+h.name);TextsCP.WriteLn();
			(*i:=0;
			WHILE h.name[i]#0X DO TextsCP.Write(h.name[i]);INC(i);END;TextsCP.WriteLn();
			*)
		END;
		h:=h.suc;
	END (*while*);
	RETURN ~error;
END checkSyntax;
            
PROCEDURE (ebnf:EBNF)Compile*():BOOLEAN,NEW; 
VAR ok:BOOLEAN;
BEGIN 
	
	(*set R to the beginning of the text to be compiled*) 
	(*TextsCP.WriteString("Compile Start read Grammar");TextsCP.WriteLn();
	R.filename:= texts.Texts.grammar;	
	TextsCP.OpenReader(R);*)
	NEW(ebnf.EbnfReader);
	ebnf.EbnfReader.openEbnf();
	TextsCP.WriteString("EBNF nach OpenReader");TextsCP.WriteLn();	
	
	ok:=FALSE;
    ebnf.lastpos := 0; 
    NEW(ebnf.sentinel);ebnf.list:=ebnf.sentinel;ebnf.h:=ebnf.list;
    ebnf.ch:=ebnf.EbnfReader.readEbnfChar(); 
    ebnf.GetSym;
    ebnf.syntax;  
    IF ebnf.checkSyntax() THEN ok:=TRUE;
    END;   
    (*TextsCP.Append(Oberon.Log,W.buf) *)   
    IF ok THEN
    	TextsCP.WriteString("Compile ok")
    ELSE TextsCP.WriteString("Compile failed");
    END;
    TextsCP.WriteLn(); 
    RETURN ok; 
END Compile;  


PROCEDURE (ebnf:EBNF)setChild(mother,child:tree.TreeNode),NEW;
VAR name:RTS.NativeString;
BEGIN
	TextsCP.WriteString("setChild:");
	IF mother # NIL THEN
		name:=mother.nodeName();
		TextsCP.WriteString(" mother: "+name);
		IF child # NIL THEN
			name:=child.nodeName();
			TextsCP.WriteString("child: "+name);
		END;
		mother(tree.NonTerminalTreeNode).child:=child;
	END;
	TextsCP.WriteLn;
END setChild;

PROCEDURE (ebnf:EBNF)setSuc(prev,suc:tree.TreeNode),NEW;
VAR name:RTS.NativeString;
BEGIN
	TextsCP.WriteString("setSuc:");		
	IF prev # NIL THEN
		name:=prev.nodeName();
		TextsCP.WriteString("prev: "+name);
		IF suc # NIL THEN
			name:=suc.nodeName();
			TextsCP.WriteString(" suc: "+name);
		END;
		prev.suc:=suc;
	END;
	TextsCP.WriteLn;
END setSuc;


PROCEDURE (ebnf:EBNF)parse(mother:tree.NonTerminalTreeNode;
				prev:tree.TreeNode;
				node:Symbol):BOOLEAN,NEW;

VAR resParse:BOOLEAN; 
	treeNode:tree.TreeNode;
	pos:INTEGER;
	nodeName:ARRAY IdLen OF CHAR;
	element,dummyElement:Element;
	

		
		PROCEDURE match(tNode:Terminal):BOOLEAN;
	
		VAR resMatch:BOOLEAN;elementInMatch:Element;
			terminalTreeNode:tree.TerminalTreeNode;
			
			
			
		(*          *)
		BEGIN (*match*)
			TextsCP.WriteString("match:  Start pos: ");
			TextsCP.WriteInt(ebnf.shared.getSharedText().getParsePos(),2);
			TextsCP.WriteString(" "+tNode.name);
			TextsCP.WriteLn();
			IF ebnf.shared.backTrack THEN
			
				TextsCP.WriteString
				("match: shared.backTrack 1 true return false");
				TextsCP.WriteLn();
				RETURN FALSE;
			END;
			
			(* empty tNode for repetition ('{....}')
			or alternative ('[....]') *)
			IF tNode.reg=NIL THEN
				IF (tNode.name="") & ((tNode.sym=rbrak) OR 
					(tNode.sym = rbrace)) THEN resMatch:=TRUE;
				ELSE RTS.Throw(" error in Grammar match"); 
					resMatch:=FALSE;
				END;
			ELSE
				resMatch:=
				RegexMatching.EditMatch(tNode.reg.regex,ebnf.shared);
				(*tNode.regexCompiled.editMatch(shared);*)
			END;
			IF resMatch THEN 
				TextsCP.WriteString("match: after EditMatch resMatch true");
			ELSE
				TextsCP.WriteString("match: after EditMatch resMatch false");
			END; 
			
			TextsCP.WriteString(" for "+tNode.name+ " parsePos: ");
			TextsCP.WriteInt(ebnf.shared.getSharedText().getParsePos(),2); 
			TextsCP.WriteLn();
			IF ebnf.shared.backTrack THEN  
				TextsCP.WriteString
				("match: EditMatch shared.backTrack 2 true return false");				
				TextsCP.WriteLn();
				RETURN FALSE 
			ELSIF resMatch THEN
				(* bredth second, get successor of some (previous) rule*)
				elementInMatch:=ebnf.parseStack.pop();
				IF elementInMatch = NIL (* termination *) THEN
					IF ebnf.shared.backTrack THEN 
						TextsCP.WriteString
						("match: shared.backTrack 3 true return false");				
						TextsCP.WriteLn();
						RETURN FALSE;
					ELSE  
						TextsCP.WriteString
						("match: success elementInMatch NIL termination ");				
						TextsCP.WriteLn();
						terminalTreeNode:=
							tree.TerminalTreeNode.TerminalTreeNodeFactory(MKSTR(tNode.name),
							ebnf.shared.getSharedText(),
							pos,
							ebnf.shared.getSharedText().getParsePos());
						treeNode:=terminalTreeNode;
						ebnf.setChild(mother,treeNode);
						ebnf.setSuc(prev,treeNode);
						RETURN TRUE;
					END;
				ELSE 
					terminalTreeNode:=
					tree.TerminalTreeNode.TerminalTreeNodeFactory(MKSTR(tNode.name),
					ebnf.shared.getSharedText(),
					pos,ebnf.shared.getSharedText().getParsePos());
					
					treeNode:=terminalTreeNode;
					ebnf.setChild(mother,treeNode);
					ebnf.setSuc(prev,treeNode);		
					(* if sequence of more than one terminal node
					*)			
					IF elementInMatch.treeNode=NIL THEN
						TextsCP.WriteString
						("match elementInMatch.treeNode=NIL for preceding"+
						tNode.name);
						TextsCP.WriteLn;
						elementInMatch.treeNode:=treeNode;
					ELSE TextsCP.WriteString("****match elementInMatch.treeNode#NIL");
						TextsCP.WriteLn;
					END;
					
					resMatch:=ebnf.parse(NIL,
					elementInMatch.treeNode,
					elementInMatch.symbol.next); 
					(* redo stack *)
					ebnf.parseStack.push(elementInMatch);
					IF ebnf.shared.backTrack THEN 
						TextsCP.WriteString
						("match: shared.backTrack 4 true return false");				
						TextsCP.WriteLn();
						treeNode:=NIL;
						ebnf.setChild(mother,NIL);
						ebnf.setSuc(prev,NIL);
						RETURN FALSE;
					END;
					IF resMatch THEN
						RETURN TRUE;
					ELSE treeNode:=NIL; 
						ebnf.setChild(mother,NIL);
						ebnf.setSuc(prev,NIL);
						RETURN FALSE;
					END;
				END;
			ELSE RETURN FALSE;
			END;		
		END match;
	
		
	


BEGIN (*parse*)
	INC(ebnf.count);
	TextsCP.WriteString("@parse Entry count: ");
	TextsCP.WriteInt(ebnf.count,2);
	TextsCP.WriteLn();
	getNodeName(node,nodeName);
	TextsCP.WriteString("@parse "+nodeName);
	TextsCP.WriteLn();
	(*IF count > 20 THEN RETURN TRUE;END;*)
	treeNode:=NIL;
	
	(*  grammar error *)
	IF node = NIL THEN 
		RTS.Throw(" error node Nil"); 
		RETURN FALSE;		
	END;
	
							 
	(*tree.SyntaxTree.walker(iNode, root,NIL);*)
	
	(*  *)
	element:=NIL;
	IF node.next # NIL THEN
		TextsCP.WriteString("@parse push element for next");TextsCP.WriteLn;
		element:=ebnf.parseStack.Factory(node);
		ebnf.parseStack.push(element);
	ELSE TextsCP.WriteString("@parse Node.next NIL");	
		TextsCP.WriteLn;
	END;
	
	pos:=ebnf.shared.getSharedText().getParsePos();
	IF pos> (****texts.Shared.maxPosInParse*) ebnf.shared.getMaxPos() THEN 
		(****texts.Shared.maxPosInParse := pos;*)
		ebnf.shared.setMaxPos(pos);
	END;
	TextsCP.WriteString("@parse pos ");
	TextsCP.WriteInt(pos,2);
	TextsCP.WriteLn();
	resParse:=FALSE;	
	(* evaluate resParse;
		two possibilities: 1.terminal (i.e. bredth second after match)
						   2.nonterminal 
	*)
	IF node IS Terminal THEN
			TextsCP.WriteString("@parse terminal node");
			TextsCP.WriteLn();
			resParse:=match(node(Terminal));
			
			TextsCP.WriteString("@parse resParse after match Pos: ");
			TextsCP.WriteInt(ebnf.shared.getSharedText().getParsePos(),2);
			IF resParse THEN TextsCP.WriteString(" TRUE")
			ELSE TextsCP.WriteString(" FALSE");
			END;
			TextsCP.WriteLn();	
			
	ELSE (* nonterminal: depth first recursion *)
			
		TextsCP.WriteString("@parse nonterminal node");
		TextsCP.WriteLn();
		treeNode:=
		tree.NonTerminalTreeNode.NonTerminalTreeNodeFactory(MKSTR(nodeName));
		ebnf.setChild(mother,treeNode);
		ebnf.setSuc(prev,treeNode);
		(* set root for output *)
		IF ebnf.root=NIL THEN ebnf.root:=treeNode;
		END;
		
		IF element # NIL THEN
			element.treeNode:=treeNode;
		END;
		
		resParse:=ebnf.parse(treeNode(tree.NonTerminalTreeNode),NIL,
		node(Nonterminal).ptrToHeaderList(*pointer to headerlist*).entry);
		IF resParse THEN TextsCP.WriteString("@parse resParse true")
		ELSE TextsCP.WriteString("@parse resParse false");
			treeNode:=NIL;
		END;
		TextsCP.WriteLn();
	END;
	(* remove pushed (last) element *)
	IF element#NIL THEN 
		element.treeNode:=NIL;
		dummyElement:=ebnf.parseStack.pop();
	END;
	(* reset position *)
	ebnf.shared.getSharedText().setParsePos(pos);
	DEC(ebnf.count);
	IF resParse THEN
		RETURN TRUE;
	ELSE
	
	    (*IF shared.backTrack THEN*) (*resParse false; shared.backTrack *)
	    treeNode:=NIL;
		TextsCP.WriteString("@parse backTrack");
		TextsCP.WriteLn();
		ebnf.setChild(mother,NIL);
		ebnf.setSuc(prev,NIL);
		IF node # ebnf.startsymbol THEN 
			IF ebnf.shared.backTrack THEN RETURN FALSE
			ELSIF node.alt=NIL (* check whether alternative *) THEN
				TextsCP.WriteString("@parse node.alt=Nil");
				TextsCP.WriteLn();
				RETURN FALSE
			ELSE 
				TextsCP.WriteString("@parse vor parse node.alt");
				TextsCP.WriteLn();
				(* todo fehler wenn mother und prev # nil???*)
				resParse:=ebnf.parse(mother,prev,node.alt);
				RETURN resParse;
			END;
		ELSE (* node = startsymbol*)
			(* wait, until caret is reset (caretPos < (errorposition:) maxPosInParse) *)
			ebnf.shared.backTrack:=FALSE;
			(*****WHILE (shared.errorCase(texts.Shared.maxPosInParse)) DO ***)
			WHILE (ebnf.shared.errorCase(ebnf.shared.getMaxPos())) DO
				
			END;
			ebnf.shared.getSharedText().setParsePos(0);
			TextsCP.WriteString("@parse after backtrack restart");
			TextsCP.WriteLn();
			(*shared.backTrack:=FALSE;*)
			(* new start *)
			treeNode:=NIL;ebnf.parseStack.ptr:=NIL;
			ebnf.setChild(mother,NIL);
			ebnf.setSuc(prev,NIL);
			 (*texts.Shared.maxPosInParse:=0;*)
			ebnf.count:=0;element:=NIL;ebnf.root:=NIL;
			resParse :=ebnf.parse(NIL,NIL,node);
			TextsCP.WriteString("@parse after restart ");
			IF resParse THEN TextsCP.WriteString("resParse TRUE")
			ELSE TextsCP.WriteString("resParse FALSE");
			END;
			TextsCP.WriteLn();
			RETURN resParse;
		END;
	
	END;	
	
END parse;



PROCEDURE (ebnf:EBNF)syntaxDrivenParse*():tree.TreeNode,NEW;


BEGIN
	ebnf.root:=NIL;
	IF ebnf.parse(NIL,NIL,ebnf.startsymbol) THEN
		tree.SyntaxTree.walker(ebnf.iNode, ebnf.root,NIL);
		TextsCP.WriteLn;
		TextsCP.WriteString("Parse true");
		TextsCP.WriteLn;
		
		RETURN ebnf.root;
	ELSE RETURN NIL;
	END;	
END syntaxDrivenParse;


PROCEDURE (ebnf:EBNF)init*(sh:texts.Shared):BOOLEAN,NEW;

VAR ch:CHAR;R:TextsCP.Reader;  

BEGIN
	
	ebnf.count:=0;ebnf.startsymbol:=NIL;
	NEW(ebnf.iNode);
	NEW (ebnf.parseStack);ebnf.parseStack.ptr:=NIL;ebnf.root:=NIL;
	TextsCP.WriteString("Init entry");TextsCP.WriteLn();
	TextsCP.WriteString("vor RegexReplace.Init");
	TextsCP.WriteLn();
	TextsCP.WriteString("Init read RegexReplace");TextsCP.WriteLn();
	
	
	TextsCP.WriteString("EBNF nach OpenReader");TextsCP.WriteLn();	
	(*RegexReplace.Init();*)	
	
	IF ebnf.Compile() THEN 		
		TextsCP.WriteString("nach Compile");TextsCP.WriteLn();		
		NEW(ebnf.startsymbol);
		ebnf.startsymbol.alt:=NIL;ebnf.startsymbol.next:=NIL;	
		ebnf.startsymbol.ptrToHeaderList:=ebnf.list;
		ebnf.shared:=sh;
		
		RETURN TRUE;
	ELSE 
		TextsCP.WriteString("Compile false");TextsCP.WriteLn();
		THROW("compile error"); RETURN FALSE;
	END;
	
END init;

BEGIN (*Auto-generated*)
	
	NEW(ebnf);
	
	(******************************)
	ebnf.count:=0;
	ebnf.shared:=NIL;ebnf.startsymbol:=NIL;
	(*texts.Shared.maxPosInParse:=0;*)
	NEW (ebnf.parseStack);ebnf.parseStack.ptr:=NIL;
	TextsCP.WriteString("EBNF Start ");TextsCP.WriteLn();
	
		
	IF ebnf.init(ebnf.shared) THEN 		
		TextsCP.WriteString("EBNF nach Init");TextsCP.WriteLn();			
		(*  *)
		(*txt:=shared.texts;*)
		
		IF ebnf.parse(NIL,NIL,ebnf.startsymbol(* before: list only *)) THEN
			TextsCP.WriteString(" parse ok")
		ELSE TextsCP.WriteString(" parse failed");
			(* return errorposition TO DO *)
			(*TextsCP.WriteString(" maxPosInParse: ");
			TextsCP.WriteInt(texts.Shared.maxPosInParse,2);
			*)
		END;
		
	END;
	
	
	TextsCP.WriteString("EBNF End");TextsCP.WriteLn();
	(*************************************************************************)
END Ebnf1.


