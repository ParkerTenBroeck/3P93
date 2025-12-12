import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Objects;
import java.util.function.Function;

public class Main {
    public enum Kind implements Comparable<Kind>{
        Single,
        Mp,
        Mpi,
    }

    public enum Scene implements Comparable<Scene>{
        Bricks,
        Halo,
        Wavy
    }
    public record Run(Scene scene, int resolution, Kind kind, boolean fs, int parallelism, double render_time, double total_time) implements Comparable<Run> {


        @Override
        public int compareTo(Run o) {
            if(fs != o.fs) return Boolean.compare(fs, o.fs);
            if (scene != o.scene) return scene.compareTo(o.scene);
            if (resolution != o.resolution) return Integer.compare(resolution, o.resolution);
            if (parallelism != o.parallelism) return Integer.compare(parallelism, o.parallelism);
            if (kind != o.kind) return kind.compareTo(o.kind);
            return 0;//Double.compare(time, o.time);
        }

        public boolean myEquals(Run run) {
            return fs == run.fs && resolution == run.resolution && parallelism == run.parallelism && (kind == run.kind) && scene == run.scene;
        }

        @Override
        public boolean equals(Object o) {
            if (o == null || getClass() != o.getClass()) return false;
            Run run = (Run) o;
            return fs == run.fs && resolution == run.resolution && parallelism == run.parallelism && kind == run.kind && scene == run.scene;
        }

        @Override
        public int hashCode() {
            return Objects.hash(scene, resolution, kind, fs, parallelism);
        }
    }

    static List<Run> loadRuns(String path) throws IOException{
        return Files.readAllLines(Path.of(path)).stream().filter(s -> !s.isBlank()).map(line -> {
            var parts = line.split(",", 3);
            var render_time = Double.parseDouble(parts[1].trim());
            var total_time = Double.parseDouble(parts[2].trim());

            var id_parts = parts[0].trim().split("_");
            boolean file = id_parts[id_parts.length - 1].equals("f");

            String threads = "";
            Kind kind;
            if(id_parts[0].startsWith("mpi")){
                kind = Kind.Mpi;
                threads = id_parts[0].substring("mpi".length());
            }else if(id_parts[0].startsWith("mp")){
                kind = Kind.Mp;
                threads = id_parts[0].substring("mp".length());
            }else {
                kind = Kind.Single;
            }

            var parallism = threads.isBlank()?1:Integer.parseInt(threads);

            Scene scene;
            if(id_parts[1].equals("halo")){
                scene = Scene.Halo;
            }else if(id_parts[1].equals("bricks")){
                scene = Scene.Bricks;
            }else if(id_parts[1].equals("wavy")){
                scene = Scene.Wavy;
            }else{
                throw new RuntimeException(parts[0]);
            }

            int resolution = Integer.parseInt(id_parts[2]);

            return new Run(scene, resolution, kind, file, parallism, render_time, total_time);
        })
                .sorted()
                .toList();
    }

    record TwoTuple<T, V>(T _0, V _1) { }

    static Main.Run find(Run run, List<Run> runs){
        return runs.stream().filter(run::myEquals).findFirst().get();
    }

    public static String generate_table(List<Main.Run> runs, Kind kind, Function<Main.Run, Double> time){
        var table = new LatexTable("l|c c c c");
        table.entry("");


        for(var parallelism: new int[]{2, 4, 8, 16}){
            table.entry(parallelism+"");
        }
        table.nextRow();
        table.hline();
        for(var scene: new Scene[]{Scene.Bricks, Scene.Halo, Scene.Wavy}){
            for(var resolution: new int[]{360, 720, 1080}){
                table.entry(scene.name() + " " + resolution);
                for(var parallelism: new int[]{2, 4, 8, 16}){
                    var r1 = time.apply(find(new Run(scene, resolution, kind, false, 1, 0.0, 0.0), runs));
                    var rp = time.apply(find(new Run(scene, resolution, kind, false, parallelism, 0.0, 0.0), runs));
                    table.entry(String.format("\\SE{%.2f}{%.2f}", (r1/rp), 100*(r1/(rp*parallelism))));
                }
                table.nextRow();
            }
        }
        table.entry("Average");
        for(var parallelism: new int[]{2, 4, 8, 16}){
            int count = 0;
            double speedup = 0;
            double effic = 0;
            for(var scene: new Scene[]{Scene.Bricks, Scene.Halo, Scene.Wavy}) {
                for (var resolution : new int[]{360, 720, 1080}) {
                    count++;
                    var r1 = time.apply(find(new Run(scene, resolution, kind, false, 1, 0.0, 0.0), runs));
                    var rp = time.apply(find(new Run(scene, resolution, kind, false, parallelism, 0.0, 0.0), runs));
                    speedup += r1/rp;
                    effic += 100*(r1/(rp*parallelism));
                }
            }
            table.entry(String.format("\\SE{%.2f}{%.2f}", speedup/count, effic/count));
        }
        table.nextRow();

        table.end();
        return "\\newcommand{\\SE}[2]{\\footnotesize \\(\\begin{aligned}S&=#1\\times \\\\[-5pt] E&=#2\\% \\end{aligned}\\)}\n"+table;
    }

    public static void main(String... args) throws IOException {
        var runs = loadRuns("data.txt");

        Files.writeString(Path.of("table_mpi_all.tex"), generate_table(runs, Kind.Mpi, Run::total_time));
        Files.writeString(Path.of("table_mp_all.tex"), generate_table(runs, Kind.Mp, Run::total_time));

        Files.writeString(Path.of("table_mpi_render.tex"), generate_table(runs, Kind.Mpi, Run::render_time));
        Files.writeString(Path.of("table_mp_render.tex"), generate_table(runs, Kind.Mp, Run::render_time));

        Files.writeString(Path.of("graph_wavy_all.tex"), LatexGraph.graph(runs, Scene.Wavy, Run::total_time));
        Files.writeString(Path.of("graph_halo_all.tex"), LatexGraph.graph(runs, Scene.Halo, Run::total_time));
        Files.writeString(Path.of("graph_bricks_all.tex"), LatexGraph.graph(runs, Scene.Bricks, Run::total_time));

        Files.writeString(Path.of("graph_wavy_render.tex"), LatexGraph.graph(runs, Scene.Wavy, Run::render_time));
        Files.writeString(Path.of("graph_halo_render.tex"), LatexGraph.graph(runs, Scene.Halo, Run::render_time));
        Files.writeString(Path.of("graph_bricks_render.tex"), LatexGraph.graph(runs, Scene.Bricks, Run::render_time));
    }
}
