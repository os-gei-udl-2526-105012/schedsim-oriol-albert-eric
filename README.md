# SCHSIM - Pràctica 3

## Descripció
Aquest projecte es realitza en el marc de l'assignatura de Sistemes Operatius. L'objectiu és ajudar a entendre els algorismes de planificació de la CPU i millora les habilitades de codificació en C.

Aquest projecte ha estat desenvolupat conjuntament per **Eric Buenavida, Albert Mas i Oriol Escolà**, tot i que tots els commits han estat fets per **Oriol Escolà**.

## Algoritmes
1. **FCFS (First-Come, First-Served)**  
   - Els processos s’executen en l’ordre en què arriben.  
   - És un algorisme senzill i no preemptiu, amb temps d’espera variable segons l’ordre d’arribada.

2. **Priorities (Prioritat)**  
   - Cada procés té una prioritat assignada i el CPU s’atorga al procés amb major prioritat.  
   - Pot ser **preemptiu** (un procés de major prioritat pot interrompre l’executant) o **no preemptiu** (un procés ja en execució no s’interromp).

3. **SJF (Shortest Job First)**  
   - Executa primer el procés amb la **ràfega de CPU més curta**.  
   - Té versions **preemptiva** i **no preemptiva**.  
   - Optimitza el temps mitjà d’espera, però pot provocar inanició de processos llargs.

4. **RR (Round Robin)**  
   - Assigna el CPU a cada procés per un **quantum de temps fix** de manera cíclica.  
   - És preemptiu per definició i garanteix que tots els processos tinguin accés al CPU de manera equitativa.  
   - El quantum determina l’equilibri entre temps de resposta i overhead de canvi de procés.

## Com fer-ho servir
```sh
make
./main -a fcfs -m preemptive -f ./process.csv 
./main -a fcfs -m nonpreemptive -f ./process.csv 
./main -a priorities -m preemptive -f ./process.csv 
./main -a priorities -m nonpreemptive -f ./process.csv 
./main -a sjf -m preemptive -f ./process.csv 
./main -a sjf -m nonpreemptive -f ./process.csv -
./main -a rr -m preemptive -f ./process.csv 
./main -a rr -m nonpreemptive -f ./process.csv 
```

---
**Fet per:** Eric Buenavida, Albert Mas i Oriol Escolà
