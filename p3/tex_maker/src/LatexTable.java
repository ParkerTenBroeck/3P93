/**
 * A builder for a latex table
 */
public class LatexTable {
    StringBuilder table;
    boolean first;

    public LatexTable(int len) {
        this("c|".repeat(Math.max(0, len)));
    }

    public LatexTable(String fmt) {
        this.table = new StringBuilder();
        this.table.append("\\begin{tabular}{");
        this.table.append(fmt);
        this.table.append("}\n");
        this.first = true;
    }

    public void hline(){
        if(!first) throw new RuntimeException();
        this.table.append("\\hline\n");
    }

    public void entry(String value) {
        if (!first) this.table.append("&");
        this.table.append(value);
        this.first = false;
    }

    public void nextRow() {
        this.first = true;
        this.table.append("\\\\\n");
    }


    public void end() {
        this.table.append("\\end{tabular}");
    }

    @Override
    public String toString() {
        return table.toString();
    }
}