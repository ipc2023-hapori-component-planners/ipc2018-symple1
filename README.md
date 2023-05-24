# Symple
Symple performs A* search based on Edge-Valued Multi-Valued
Decision Diagrams (EVMDDs). It supports zero, unit, constant and
state-dependent action costs.


Symple is is built on top of
[Fast Downward](http://www.fast-downward.org/),
[SymBa](https://fai.cs.uni-saarland.de/torralba/software.html)
and [MEDDLY-0.18](https://meddly.sourceforge.io/).
Therfore, it it is made available under the GNU Public License (GPL).

## 1. Compiling Symple
The command
```sh
$ git clone https://gkigit.informatik.uni-freiburg.de/dspeck/symple.git <dirname>
```
will create a clone of Symple master repository in directory "dirname".

To build Symple run the following command.
```sh
$ cd <dirname>
$ ./build
```

## 2. Running Symple
Symple is callable with several configurations.

### 2.1 Defaul Configuration (IPC-18)
Starting Symple with defaul configurations (bidirectional search, 100k node
limit per transition relation):
```sh
$ ./plan <domain> <problem> <plan-output-file>
```

### 2.2 Predefined Configurations
Starting Symple with predefinied configurations:
```sh
$ .src/plan-ipc <planner> <domain> <problem> <plan-output-file>
```
You can choose between the following predefined configurations for the "planer" 
parameter, where the term stands for the search direction and the number for 
the node limit of the transition relations.

+ **Bidirectional Search**: symple, symple0, symple10000, symple25000, symple50000, symple100000, symple200000
+ **Progression**: symplePro, symplePro0, symplePro10000, symplePro25000, symplePro50000, symplePro100000, symplePro200000
+ **Regression**: sympleReg, sympleReg0, sympleReg10000, sympleReg25000, sympleReg50000, sympleReg100000, sympleReg200000

### 2.3 Userdefined Configurations
In general, it is possible to define new configurations for Symple. The easiest
way is to insert a new predined configuration to the [downward](src/search/downward) file.
For more information, please visit the [Fast Downward](http://www.fast-downward.org/)
website.

## 3 Benchmarks
+ [Here](https://bitbucket.org/planning-researchers/classical-domains), you can find a benchmark set containing of former IPC domains.
+ [Here](https://gkigit.informatik.uni-freiburg.de/dspeck/SDAC-Benchmarks), you can find a benchmark set containing of domains with state-dependent action costs.
