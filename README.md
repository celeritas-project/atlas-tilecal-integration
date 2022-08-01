# ATLTileCalTB
A Geant4 simulation of the ATLAS Tile Calorimeter beam tests.

<figure>
<img src="./images/TileCal_mu.gif" alt="Trulli" style="width:100%">
<figcaption align="center"><b>Fig. - 10 GeV muon passing through the ATLAS TileCal.</b></figcaption>
</figure>

<br/><br/>

<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#project-description">Project description</a></li>
    <li><a href="#authors-and-contacts">Authors and contacts</a></li>
    <li>
      <a href="#how-to">How to</a>
      <ul>
        <li><a href="#build-compile-and-execute-on-maclinux">Build, compile and execute on Mac/Linux</a></li>
        <li><a href="#build-compile-and-execute-on-lxplus">Build, compile and execute on lxplus</a></li>
        <li><a href="#submit-a-job-with-htcondor-on-lxplus">Submit a job with HTCondor on lxplus</a></li>
      </ul>
    </li>
    <li><a href="#selected-atlas-tilecal-references">Selected ATLAS TileCal references</a></li>
  </ol>
</details>

<!--Project desription-->
## Project description
The project targets a standalone Geant4 simulation of the ATLAS Tile Calorimeter beam tests to perform Geant4 regression testing, physics lists comparison and validation against test-beam data.
- ⏰ Start date: 23 May 2023 
- 📌 Status: under development
  
<!--Authors and contacts-->
## Authors and contacts
- 👨‍🔬 Lorenzo Pezzotti (CERN EP-SFT) - lorenzo.pezzotti@cern.ch 
- 👨‍🔬 Stephan Lachnit (CERN EP-SFT Summer Student) - stephan.lachnit@cern.ch
- 👨‍🔬 Supervisor: Alberto Ribon (CERN EP-SFT)

<!--How to-->
## How to

### Build, compile and execute on Mac/Linux
1.  git clone the repo
    ```sh
    git clone https://github.com/lopezzot/ATLTileCalTB.git
    ```
2.  source Geant4 env
    ```sh
    source /relative_path_to/geant4.10.07_p03-install/bin/geant4.sh
    ```
3.  cmake build directory and make (using geant4.10.07_p03)
    ```sh
    mkdir ATLTileCalTB-build; cd ATLTileCalTB-build/
    cmake -DGeant4_DIR=/absolute_path_to/geant4.10.07_p03-install/lib/Geant4-10.7.3/ relative_path_to/ATLTileCalTB/
    make
    ```
4.  execute (example with TBrun.mac macro card, 2 threads and FTFP_BERT physics list)
    ```sh
    ./ATLTileCalTB -m TBrun.mac -t 2 -p FTFP_BERT
    ```

Parser options
- `-m macro.mac`: pass a Geant4 macro card (example `-m ATLHECTB_run.mac` available in source directory and automatically copied in build directory) 
- `-t integer`: pass number of threads for multi-thread execution (example `-t 2`, default is the number of threads on the machine)
- `-p Physics_List`: select Geant4 physics list (example `-p FTFP_BERT`)

### Build, compile and execute on lxplus
1. git clone the repo
   ```sh
   git clone https://github.com/lopezzot/ATLTileCalTB.git
   ```
2. cmake build directory and make (using geant4.10.07_p03, check for gcc and cmake dependencies for other versions)
   ```sh
   mkdir ATLTileCalTB-build; cd ATLTileCalTB-build/
   cp ../ATLTileCalTB/scripts/ATLTileCalTB_cvmfs_setup.sh ../ATLTileCalTB/scripts/ATLTileCalTB_lxplus_10.7.p03.sh .
   source ./ATLTileCalTB_lxplus_10.7.p03.sh -DBUILD_ANALYSIS=OFF
   ```
3. execute (example with TBrun.mac macro card, 4 threads and FTFP_BERT physics list)
   ```sh
   ./ATLTileCalTB -m TBrun.mac -t 4 -p FTFP_BERT
   ```
   
### Submit a job with HTCondor on lxplus
1. [First follow the build instructions on lxplus](#build-compile-and-execute-on-lxplus)
2. prepare for HTCondor submission (example with Geant4.10.07_p03, TBrun.mac, 4 threads, FTFP_BERT physics list)
    ```sh
    mkdir -p error log output
    cp ../ATLTileCalTB/scripts/ATLTileCalTB_HTCondor.sub ../ATLTileCalTB/scripts/ATLTileCalTB_HTCondor_10.7.p03.sh .
    sed -i "2 i cd $(pwd)" ATLTileCalTB_HTCondor_10.7.p03.sh
    echo ./ATLTileCalTB -m TBrun.mac -t 4 -p FTFP_BERT >> ATLTileCalTB_HTCondor_10.7.p03.sh
    sed -i "1 i executable = ATLTileCalTB_HTCondor_10.7.p03.sh" ATLTileCalTB_HTCondor.sub
    ```
3. submit a job
   ```sh
   condor_submit ATLTileCalTB_HTCondor.sub 
   ```
4. monitor the job
   ```sh
   condor_q
   ```
   or (for persistency)
   ```sh
   condor_wait -status log/*.log
   ```
5. additional info from HTCondor (optional) \
   rm all your jobs
    ```sh
   condor_rm username
   ```
   inspect your accounting group
   ```sh
   condor_q owner $LOGNAME -long | grep '^AccountingGroup' | sort | uniq -c
   ```
   display all accounting groups
   ```sh
   haggis group list
   ```
   display your accounting groups
    ```sh
   haggis rights
   ```
   check what accounting group a job has
   ```sh
   condor_q jobid.0 -af AccountingGroup
   ```
   specify the accounting group for yout job, to be added in the .sub file
   ```sh
   +AccountingGroup = "group_u_*"
   ```
   check job CPU usage
   ```sh
   condor_q -l jobid.0 | grep CPUsUsage
   ```
   ssh to machine where job is running
   ```sh
   condor_ssh_to_job jobid.0
   ```

<!--Selected ATLAS TileCal references-->
## Selected ATLAS TileCal references
- 📄 <em>Study of energy response and resolution of the ATLAS Tile Calorimeter to hadrons of energies from 16 to 30 GeV</em>, Eur. Phys. J. C (2021) 81:549: [![Website shields.io](https://img.shields.io/website-up-down-green-red/http/shields.io.svg)](https://link.springer.com/article/10.1140/epjc/s10052-021-09292-5)
