import java.util.Comparator;
import java.util.List;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * A utility to convert a GA configurations collected data into a latex graph
 */
public class LatexGraph {

    interface IntObjectFunction<T, R>{
        R apply(int i, T v);
    }



    static <T, R> Stream<R> enumerate(Stream<T> in, IntObjectFunction<T,R> meow){
        return in.map(new Function<T, R>() {
            int count = 0;
            @Override
            public R apply(T t) {
                return meow.apply(count++, t);
            }
        });
    }

    static String beep(List<Main.Run> runs, Main.Scene scene, Main.Kind kind, int resolution, java.util.function.Function<Main.Run, Double> mapping){
        return enumerate(
                runs.stream()
                        .filter(r -> r.scene() == scene)
                        .filter(r -> r.kind() == kind || (r.kind() == Main.Kind.Single && kind== Main.Kind.Mp))
                        .filter(r -> r.resolution() == resolution)
                        .sorted(Comparator.comparingInt(Main.Run::parallelism)),
        (i, r) -> "("+i+","+mapping.apply(r)+")")
            .collect(Collectors.joining());
    }


    public static String graph(List<Main.Run> runs, Main.Scene scene, Function<Main.Run, Double> time) {


        double min = 0;//runs.stream().filter(run -> run.scene() == scene).mapToDouble(time::apply).min().getAsDouble();
        double max = runs.stream().filter(run -> run.scene() == scene).mapToDouble(time::apply).max().getAsDouble();
        int run_count = 5-1;
        var str = """
\\begin{tikzpicture}[font=\\tiny]
\\pgfplotsset{
    xmin=0, xmax="""+run_count+"""
,xticklabels={$0$,$1$,$2$,$4$,$8$,$16$},
 ytick distance=10,\s
}

\\begin{axis}[
    axis y line*=left,
    ymin="""+min+"""
    , ymax="""+max+"""
    ,
    xlabel=parallelism,
    width=\\textwidth,
    xlabel style={at={(ticklabel* cs:0.5,3)},anchor=south},
    ylabel=time (s),
    ylabel style={at={(ticklabel* cs:0.5,-10)},anchor=north},
    xmajorgrids=true,
    ymajorgrids=true,
    grid style=dashed,
    legend columns=3,
    legend style={nodes={scale=0.5}, at={(0.5,1.02)}, anchor=south},
    legend cell align={left}
    ]

    \\addlegendimage{/pgfplots/refstyle=mp1080}\\addlegendentry{\\small 1080 MP}
    \\addlegendimage{/pgfplots/refstyle=mp720}\\addlegendentry{\\small 720 MP}
    \\addlegendimage{/pgfplots/refstyle=mp360}\\addlegendentry{\\small 360 MP}

    \\addlegendimage{/pgfplots/refstyle=mpi1080}\\addlegendentry{\\small 1080 MPI}
    \\addlegendimage{/pgfplots/refstyle=mpi720}\\addlegendentry{\\small 720 MPI}
    \\addlegendimage{/pgfplots/refstyle=mpi360}\\addlegendentry{\\small 360 MPI}

    \\addplot[very thick,blue]
    coordinates{"""+beep(runs, scene, Main.Kind.Mp, 1080, time)+"""
    }; \\label{mp1080}

    \\addplot[very thick,green]
    coordinates{"""+beep(runs, scene, Main.Kind.Mp, 720, time)+"""
    }; \\label{mp720}

    \\addplot[very thick,red]
    coordinates{"""+beep(runs, scene, Main.Kind.Mp, 360, time)+"""
    }; \\label{mp360}
    
    \\addplot[blue,dashed,very thick]
    coordinates{"""+beep(runs, scene, Main.Kind.Mpi, 1080, time)+"""
    }; \\label{mpi1080}

    \\addplot[green,dashed,very thick]
    coordinates{"""+beep(runs, scene, Main.Kind.Mpi, 720, time)+"""
    };\\label{mpi720}

    \\addplot[red,dashed,very thick]
    coordinates{"""+beep(runs, scene, Main.Kind.Mpi, 360, time)+"""
    };\\label{mpi360}
\\end{axis}
"""
                +
//"""
//\\begin{axis}[
//    axis y line=none,
//    axis x line=none,
//    ymin=0, ymax="""+labelsPerWrap+"""
//    ]
//"""
//                +
//                enumerate(item.runs.stream().sorted(Comparator.comparingInt(c -> c.generationStats.size())), (i, run) -> {
//                    var finished = run.generationStats.size()-1;
//                    var pos = (labelsPerWrap-i%labelsPerWrap)/(float)labelsPerWrap-1/(float)labelsPerWrap/2.0;
//                    return "\\addplot[thick,gray] coordinates {("
//                            +finished+",0)("
//                            +finished+","+labelsPerWrap+")}  node[black,pos="
//                            +pos+","+(finished<item.averagedStats.size()/2?"right":"left")+"]{\\tiny\\textbf{"
//                            +"G"+(run.generationStats.size()-1)+" S"+run.seed+" F"+(int)run.best.fitness+"}};\n";
//                }).collect(Collectors.joining())
//                +

//"""
//                \\end{axis}
//"""+
        """

        \\end{tikzpicture}
        """;
        return str;
    }
}