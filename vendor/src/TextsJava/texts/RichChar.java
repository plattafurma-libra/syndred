package texts;

import java.util.Arrays;

/* Font Description: 
1. Size:(e.g., 12 point vs. 16 point)
2. Style (e.g., plain vs. italic), 
3. Typeface (e.g., Times vs. Helvetica),
4. Weight (e.g., bold vs. normal),
5. Color (e.g., red ).
*/ 

	public class RichChar{
		 /* SyntaxDriven does not have
		FontDescription, i.e. always true, but syndred has*/
		public char ch;
		public char size[], style[], typeface[], weight[], 
		color[]; 
		//private Texts texts;
		
		public RichChar(char c) {
			//texts=txts;
			ch=c;
		}
		
		// SDE
		/*private boolean match(char[]one,char[]two) {
			// do not check Fontdescription for syntaxdriven--
			if  (Texts.syntaxDriven) return true;
			int pos=0;
			while(true) {
				if (one[pos]==00) {
					return (two[pos]==00);
				}
				else if (one[pos]!=two[pos]) return false;
				else pos++;
			}	
		}*/
		// syndred
		private boolean match(char[] java, char[] cp) {
			return java == null || java.length == 0 || cp == null || cp.length == 0 ? false
					: Arrays.equals(Arrays.copyOfRange(cp, 0, cp.length - 1), java);
		}
		
		public boolean size(char[]size) {
			return match(this.size,size);
		}
		
		public boolean style(char[]style) {
			return match(this.style,style);
		}
		
		public boolean typeface(char[]typeface) {
			return match(this.typeface,typeface);
		}
		
		public boolean weight(char[]weight) {
			return match(this.weight,weight);
		}
		
		public boolean color(char[]color) {
			return match(this.color,color);
		}
	

}
