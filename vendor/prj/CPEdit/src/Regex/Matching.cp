MODULE RegexMatching;

IMPORT RegexParser,FontsFont, texts, TextsCP;

TYPE Regex=POINTER TO EXTENSIBLE RECORD (RegexParser.RegexType) END;

VAR i:INTEGER;
	
	rch:texts.RichChar;
	
	(*sh:texts.Shared;*)
	
PROCEDURE WriteMessage(str:ARRAY OF CHAR);
BEGIN
	TextsCP.WriteString(str);
	TextsCP.WriteLn;
END WriteMessage;
		
PROCEDURE WriteEntry(fromProcedure:ARRAY OF CHAR;ch:CHAR;i:INTEGER);


BEGIN	
	TextsCP.WriteString(fromProcedure +" Entry:  ");		
	IF ch # 0X THEN TextsCP.Write(ch);
	END;
	
	IF i >= 0 THEN TextsCP.WriteString(" Position i: "); TextsCP.WriteInt(i,2);
	END;
	TextsCP.WriteLn();
END WriteEntry;



PROCEDURE WriteExit(fromProcedure:ARRAY OF CHAR;valResult:INTEGER;ch:CHAR;i:INTEGER);

VAR res:ARRAY 10 OF CHAR; 
BEGIN
	IF valResult=1  THEN res:="TRUE" ELSIF valResult=-1 THEN res:="FALSE" ELSE res:=""; END;
	TextsCP.WriteString(fromProcedure +" Exit: "+res+"  ");		
	IF ch # 0X THEN TextsCP.Write(ch);
	END;
	IF i >= 0 THEN TextsCP.WriteString(" Position i: ");TextsCP.WriteInt(i,2);
	END;
	TextsCP.WriteLn();
END WriteExit;

(*-------------------------Matching Procedures---------------------------*)

PROCEDURE MatchNegRange(range:RegexParser.Range; VAR flag:BOOLEAN;
sh:texts.Shared);
BEGIN
	WriteMessage("MatchNegRange Entry");
	IF sh.backTrack THEN WriteMessage("MatchNegRange backTrack");RETURN END;
	REPEAT
		flag:=((rch.ch<range.min) OR (rch.ch>range.max));
		range:=range.next;
	UNTIL range=NIL;
	WriteMessage("MatchNegRange Exit");
END MatchNegRange;

PROCEDURE MatchRange(range:RegexParser.Range; VAR flag:BOOLEAN;
sh:texts.Shared);
BEGIN
	WriteEntry("MatchRange ",rch.ch,-1);
	IF sh.backTrack THEN RETURN END;
	LOOP
		IF range=NIL THEN EXIT END;
		TextsCP.WriteString("MatchRange range.min: "); 
		TextsCP.Write(range.min);
		TextsCP.WriteString(" MatchRange range.max: "); 
		TextsCP.Write(range.max);
		TextsCP.WriteLn();
		flag:=((rch.ch>=range.min) & (rch.ch<=range.max));
		IF flag=TRUE THEN EXIT;
		ELSE range:=range.next 
		END;
	END;
	(*  are there any  subRanges *)
 	LOOP
		IF range=NIL THEN EXIT END;
		IF range.sub THEN EXIT END;
		range:=range.next
	END;
	IF range#NIL THEN MatchNegRange(range,flag,sh) END;
	
	WriteExit("MatchRange ch ",0,rch.ch,-1);
END MatchRange; 



PROCEDURE FontMatch(rch:texts.RichChar; regex:RegexParser.Regex):INTEGER;

	

BEGIN
	IF regex.Font=NIL THEN RETURN 1 
	ELSIF FontsFont.Size(regex.Font,rch) & FontsFont.Style(regex.Font,rch) & 
		FontsFont.Typeface(regex.Font,rch) &  
		FontsFont.Weight(regex.Font,rch) & FontsFont.Color(regex.Font,rch) THEN
		RETURN 1 
	ELSE RETURN -1
	END;
END FontMatch;


PROCEDURE MatchRegex(regex:RegexParser.Regex;resetPos:INTEGER;
VAR flag:BOOLEAN;sh:texts.Shared);
VAR  branch:RegexParser.Branch;
	 	j:INTEGER (*ResetPosition*);
	 	res:INTEGER;
	
	PROCEDURE MatchBranch(branch:RegexParser.Branch; VAR flag:BOOLEAN);
	VAR piece:RegexParser.Piece;
	res:INTEGER;
	
		PROCEDURE MatchPiece(piece:RegexParser.Piece; VAR flag:BOOLEAN);
		VAR atom,temp_atom:RegexParser.Atom;
				min,max (* nr repetitions *),
				q (*counts number matches for quantified or for '*' or '+'*),
				j1:INTEGER (* interim value for i; formerly j; is named here j1 for
				difference with j in MatchRegex *);
				temp_flag:BOOLEAN;
				res:INTEGER;
				
			PROCEDURE MatchAtom(atom:RegexParser.Atom; VAR flag:BOOLEAN);
			VAR range:RegexParser.Range;res:INTEGER;
			
				
			BEGIN (* MatchAtom *)
				WriteEntry("MatchAtom ",' ',-1);
				IF sh.backTrack THEN 
					WriteMessage("MatchAtom sh.backTrack RETURN");
					RETURN 
				END;
				IF atom.range=NIL THEN  
					WriteMessage("MatchAtom range Nil vor MatchRegex");
					MatchRegex(atom.regex,resetPos,flag,sh);
				ELSE	
				
					rch := sh.getSym();
							
					IF sh.backTrack THEN 
						WriteMessage("MatchAtom sh.backtrack nach getSym");
						RETURN 
					END;			
					
					TextsCP.WriteString("MatchAtom getSym ch: ");
					TextsCP.Write(rch.ch); 
					TextsCP.WriteLn();
						
					IF atom.range.pos THEN
						WriteMessage("MatchAtom range.pos vor MatchRange");
						MatchRange(atom.range,flag,sh); 
						WriteMessage("MatchAtom range.pos nach MatchRange");
					ELSE 
						WriteMessage("MatchAtom vor MatchNegRange");
						MatchNegRange(atom.range,flag,sh);
					END;
				
				END;
				IF flag THEN 
					WriteMessage("MatchAtom vor FontMatch"); 
					res:= FontMatch(rch,regex);
					WriteMessage("MatchAtom nach FontMatch");
					flag:=res=1;	
					(*flag ausschreiben	*)			
				ELSE
					res:=-1;
				END;
				
				WriteExit("MatchAtom result, ch ",res,rch.ch,-1);
			END MatchAtom;
			
		BEGIN (*MatchPiece*) (*hier Matching-Procedures aufrufen piece.MatchProcQuantified(piece,flag)*)
			(*MatchProcOptional (?)*)
			WriteEntry("MatchPiece ",0X,-1);
			IF sh.backTrack THEN RETURN END;
			flag:=FALSE;
			temp_flag:=FALSE;
			q:=0;
			(* get value of i, needed in repeat loops down case 2 and case 3 *)
			i := sh.getSharedText().getParsePos();
			CASE piece.id OF 
				1:  (* question mark, '?', optional *)
					WriteMessage("MatchPiece Case 1 Optional");
				
					atom:=piece.atom;   
					min:=0;
					max:=1;
					WriteMessage("MatchPiece Case 1 Optional vor MatchAtom");
					MatchAtom(atom,flag);
					IF sh.backTrack THEN RETURN END;
					IF ~flag THEN flag:=TRUE;
						IF atom.regex=NIL THEN DEC(i) END
					END; 
					sh.getSharedText().setParsePos(i);
		
				|2: (* repetitions, quantified (???) *)
					WriteMessage("MatchPiece Case 2 Quantifier ");
					atom:=piece.atom;   (*Quantified*)
					min:=piece.min.val;
					max:=piece.max.val;
					TextsCP.WriteString("MatchPiece min");TextsCP.WriteInt(min,2);
					TextsCP.WriteString("MatchPiece max");TextsCP.WriteInt(max,2);
					q:=0;
					
					
					j1:=i;
					REPEAT 
						WriteMessage("MatchPiece Case 2 quantified in Repeat vor MatchAtom");
						MatchAtom(atom,flag);
						i:=sh.getSharedText().getParsePos();
						IF sh.backTrack THEN 
							WriteMessage("MatchPiece in Repeat sh.backTrack");
							RETURN 
						END;
						IF flag THEN INC(q) END;
					UNTIL (~flag) OR (q=max);
					IF ~flag & (q>=min) THEN flag:=TRUE; 
						IF atom.regex=NIL THEN DEC(i) END 
					END;
					sh.getSharedText().setParsePos(i);
					
				|3: (* asterisk('*' ) or plus('+') *)
					WriteMessage("MatchPiece Case 3 * or + ");
					atom:=piece.atom; (*Unbounded*) (*max=NIL*)
					temp_atom:=piece.suc.atom;
					min:=piece.min.val;
					REPEAT 
						WriteMessage("MatchPiece case 3 * or + in Repeat vor MatchAtom 1");
						MatchAtom(atom,flag);
						IF sh.backTrack THEN RETURN END;
						i:=sh.getSharedText().getParsePos();
						(* j1: save increased i before decreasing;reset
						parsePosition *)
						j1:=i;
						DEC(i); 
						sh.getSharedText().setParsePos(i);
						WriteMessage("MatchPiece case 3 * or + in Repeat vor MatchAtom 2");
						
						MatchAtom(temp_atom,temp_flag);
						i := sh.getSharedText().getParsePos();
						IF temp_atom.regex#NIL THEN i:=j1; 
							sh.getSharedText().setParsePos(i);
						END;
						IF flag THEN INC(q) END;
						IF temp_flag THEN DEC(q); flag:=FALSE END;
					UNTIL (~flag);
					IF ~flag & (q>=min) THEN flag:=TRUE; 
						IF atom.regex=NIL THEN DEC(i) END 
					END;
					(* 							  *)
					sh.getSharedText().setParsePos(i);
		
			END (*end-case*);
			IF sh.backTrack THEN RETURN 
			END;
			IF flag THEN res:=1 ELSE res:=-1;
			END;
			
			WriteExit("MatchPiece: ",res,rch.ch,i);
		END MatchPiece; 
		
		PROCEDURE Final():BOOLEAN;
		(* JR to be refined ? *)
		BEGIN
			IF sh.backTrack THEN RETURN FALSE END;
			IF piece.suc=NIL THEN
				IF piece.atom # NIL THEN
					IF piece.atom.range#NIL THEN
						IF (piece.atom.range.min=0X) & (piece.atom.range.max=0X) THEN RETURN TRUE;
						END;
					END;
				END;					
			END;
			RETURN FALSE;
		END Final;
		
	BEGIN (*MatchBranch*)
		WriteEntry("MatchBranch ",0X,-1);
		
		piece:=branch.piece;
		LOOP 
			IF (piece=NIL) OR Final()(*JR*) THEN  EXIT; (*alle Pieces abgearbeitet und ganzen String*)
			END;
			WriteMessage("matchBranch vor MatchPiece in Loop");
			MatchPiece(piece,flag);
			WriteMessage("matchBranch nach MatchPiece in Loop");
			IF sh.backTrack THEN RETURN 
			END;
			IF flag THEN  piece:=piece.suc; 
				
			ELSE EXIT 
			END;
		END;
		IF flag THEN res:=1 ELSE res:=-1;
		END;
		WriteExit("MatchBranch ch ",res,rch.ch,-1);
	END MatchBranch;

(*TODO reset???*)
BEGIN (*MatchRegex*)
	j:=i; (* save position *)
	WriteEntry("MatchRegex ch  i (= j): ",' ',i);
	branch:=regex.branch;
	
	LOOP
		IF (branch=NIL) THEN EXIT END; 
		MatchBranch(branch,flag);
		IF sh.backTrack THEN RETURN END;
		IF flag THEN EXIT;
		ELSE 
			
			(*	
				Out.String("MatchRegex: Branch False  j= ");
			Out.Int(j,2);Out.String("ch=");Out.Char(ch);Out.Ln();
			
			*)
			
			TextsCP.WriteLn();TextsCP.WriteString("MatchRegex Branch false j:");
			TextsCP.WriteInt(j,2);TextsCP.WriteString(" ch=");
			TextsCP.Write(rch.ch);TextsCP.WriteLn();
			(* reset,
			toDo parsePos*)
			i:=j;
			(*        ch:=tarString[i];     *)
			(*****************
			ch := GetCharAtPos(i,sh); 
			***************************)
			(*sh.getCharAtTextPos(i);*)
			IF sh.backTrack THEN RETURN END;
			branch:=branch.alt
		END
	END;
	IF flag THEN res:=1 ELSE res:=-1;END;
	WriteExit("MatchRegex ch ",res,rch.ch,i);
END MatchRegex;
				
PROCEDURE Match*(regex:RegexParser.Regex;target:POINTER TO ARRAY OF CHAR):BOOLEAN;
VAR flag:BOOLEAN; branch:RegexParser.Branch;dummy:INTEGER;
BEGIN

	(*___________________________________________
	(*tarString:=target;*)
	
	flag:=FALSE;
	i:=0;
	(*ch:=tarString[i];*)
	dummy:=0;
	MatchRegex(regex,dummy,flag);
	IF sh.backTrack THEN i:=0; RETURN FALSE END;
	IF ch#0X THEN (*ch:=tarString[i]*) ch:=0X; END;
	IF (flag) & (ch#0X) THEN flag:=FALSE END;
	RETURN flag
	-----------------------------------------*)
	RETURN FALSE;
END Match; 

(* called from ebnf-parser *)
PROCEDURE EditMatch*(regex:RegexParser.Regex;shared:texts.Shared):BOOLEAN;
VAR flag:BOOLEAN;
BEGIN
	WriteEntry("RegexMatching.EditMatch ",0X,-1);
	flag:=FALSE;
	(*sh:=shared;*)
	
	i:=shared.getSharedText().getParsePos();
	
	TextsCP.WriteString("RegexMatching.EditMatch i: ");
	TextsCP.WriteInt(i,2);
	TextsCP.WriteLn;
	
	TextsCP.WriteString("RegexMatching.EditMatch TextLen: ");
	TextsCP.WriteInt(shared.getSharedText().getTextLen(),2);
	TextsCP.WriteLn;
	MatchRegex(regex,i,flag,shared);
	IF shared.backTrack THEN i:=0; 
		(* reset todo*)
		TextsCP.WriteString("RegexMatching Return false ");
		TextsCP.WriteLn;
		RETURN FALSE; 
	END;
	TextsCP.WriteString("RegexMatching Return flag ");
	IF flag THEN TextsCP.WriteString("True ")
	ELSE TextsCP.WriteString("False ");
	END;
	TextsCP.WriteLn;
	RETURN flag;
	
END EditMatch;



END RegexMatching.