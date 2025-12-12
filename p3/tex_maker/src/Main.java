import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Objects;

public class Meow {
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
    public record Run(Scene scene, int resolution, Kind kind, boolean fs, int parallelism, double time) implements Comparable<Run> {


        @Override
        public int compareTo(Run o) {
            if(fs != o.fs) return Boolean.compare(fs, o.fs);
            if (scene != o.scene) return scene.compareTo(o.scene);
            if (resolution != o.resolution) return Integer.compare(resolution, o.resolution);
            if (parallelism != o.parallelism) return Integer.compare(parallelism, o.parallelism);
            if (kind != o.kind) return kind.compareTo(o.kind);
            return Double.compare(time, o.time);
        }

        public boolean myEquals(Run run) {
            return fs == run.fs && resolution == run.resolution && parallelism == run.parallelism && (kind == run.kind) && scene == run.scene;
        }

        @Override
        public boolean equals(Object o) {
            if (o == null || getClass() != o.getClass()) return false;
            Run run = (Run) o;
            return fs == run.fs && Double.compare(time, run.time) == 0 && resolution == run.resolution && parallelism == run.parallelism && kind == run.kind && scene == run.scene;
        }

        @Override
        public int hashCode() {
            return Objects.hash(scene, resolution, kind, fs, parallelism, time);
        }
    }

    static List<Run> loadRuns(String path) throws IOException{
        return Files.readAllLines(Path.of(path)).stream().filter(s -> !s.isBlank()).map(line -> {
            var parts = line.split(" ", 2);
            var id = parts[0];
            var time = Double.parseDouble(parts[1]);

            boolean file = false;
            if (id.endsWith("-f")){
                id = id.substring(0, id.length()-2);
                file = true;
            }
            Kind kind;
            if(id.startsWith("mpi")){
                kind = Kind.Mpi;
                id = id.substring("mpi".length());
            }else if(id.startsWith("mp")){
                kind = Kind.Mp;
                id = id.substring("mp".length());
            }else {
                kind = Kind.Single;
            }

            var split = id.split("-", 2);
            var parallism = split[0].isBlank()?1:Integer.parseInt(split[0]);
            id = split[1];

            Scene scene;
            if(id.startsWith("halo")){
                scene = Scene.Halo;
                id = id.substring("halo".length());
            }else if(id.startsWith("bricks")){
                scene = Scene.Bricks;
                id = id.substring("bricks".length());
            }else if(id.startsWith("wavy")){
                scene = Scene.Wavy;
                id = id.substring("wavy".length());
            }else{
                throw new RuntimeException(id);
            }

            int resolution = Integer.parseInt(id);

            return new Run(scene, resolution, kind, file, parallism, time);
        })
                .sorted()
                .toList();
    }

    record TwoTuple<T, V>(T _0, V _1) { }

    static double find(Run run, List<Run> runs){
        return runs.stream().filter(run::myEquals).findFirst().get().time;
    }

    public static void main(String... args) throws IOException {
        var runs1 = loadRuns("data_final.txt");
//        System.out.println(LatexGraph.graph(runs1, Scene.Bricks));

        var kind = Kind.Mpi;
        var table = new LatexTable(5);
        table.entry("");


        for(var parallelism: new int[]{2, 4, 8, 16}){
            table.entry(parallelism+"");
        }
        table.nextRow();
        for(var scene: new Scene[]{Scene.Bricks, Scene.Halo, Scene.Wavy}){
            for(var resolution: new int[]{360, 720, 1080}){
                table.entry(scene.name() + " " + resolution);
                for(var parallelism: new int[]{2, 4, 8, 16}){
                    var r1 = find(new Run(scene, resolution, kind, false, 1, 0.0), runs1);
                    var rp = find(new Run(scene, resolution, kind, false, parallelism, 0.0), runs1);
                    table.entry(String.format("$\\frac{S=%.2f\\times}{E=%.2f\\%%}$", (r1/rp), 100*(r1/(rp*parallelism))));
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
                    var r1 = find(new Run(scene, resolution, kind, false, 1, 0.0), runs1);
                    var rp = find(new Run(scene, resolution, kind, false, parallelism, 0.0), runs1);
                    speedup += r1/rp;
                    effic += 100*(r1/(rp*parallelism));
                }
            }
            table.entry(String.format("$\\frac{S=%.2f\\times}{E=%.2f\\%%}$", speedup/count, effic/count));
        }
        table.nextRow();

        table.end();
        System.out.println(table.toString());
//        var runs2 = loadRuns("data2.txt");
//
//        var average = IntStream.range(0, runs1.size())
//                .mapToObj(i -> new TwoTuple<>(runs1.get(i), runs2.get(i)))
//                .filter(i -> !i._0.fs)
//                .filter(i -> i._0.kind == Kind.Mp)
//                .filter(i -> i._0.resolution == 1080)
//                .mapToDouble(t -> t._1.time/t._0.time)
//                .average()
//                .orElse(99999999)
//                    ;
//        System.out.println(average);
    }
}
