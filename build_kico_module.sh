 #!/bin/bash
mvn clean package -pl org.lflang,org.lflang.ide,org.lflang.diagram,org.lflang.targetplatform
mkdir tmp
(cd tmp; unzip -uo ../org.lflang/target/org.lflang-0.1.0-SNAPSHOT.jar)
(cd tmp; unzip -uo ../org.lflang.diagram/target/org.lflang.diagram-0.1.0-SNAPSHOT.jar)
jar -cvf kico-lf.jar -C tmp .
