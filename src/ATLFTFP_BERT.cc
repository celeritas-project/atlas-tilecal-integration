//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
//---------------------------------------------------------------------------
//
// ClassName:
//
// Author: 2007 Gunter Folger
//
//   created from FTFP
//
// Modified:
// 19.06.2008 G.Folger: don't use chips quasielastic in FTF
// 27.11.2009 G.Folger: Remobe experimental status
// 04.06.2010 G.Folger: Use new ctor for builders
// 16.08.2010 H.Kurashige: Remove inclusion of G4ParticleWithCuts
// 26.06.2012 A.Ribon:  Use FTF/Preco and BERT for nuclear capture at rest.
// 27.07.2012 A.Ribon:  Use the new class G4BertiniAndFritiofStoppingPhysics
// 16.10.2012 A.Ribon:  Renamed the physics classes used
//
//----------------------------------------------------------------------------
//
#include "ATLFTFP_BERT.hh"

#include <iomanip>

#include "globals.hh"
#include "G4ios.hh"

#include "G4DecayPhysics.hh"
#include "G4EmExtraPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4NeutronTrackingCut.hh"
#include "G4VPhysicsConstructor.hh"

#include "G4HadronPhysicsFTFP_BERT.hh"

#include "G4SystemOfUnits.hh"
#include "G4ParticleDefinition.hh"
#include "G4EmParameters.hh"
#include "G4EmBuilder.hh"
#include "G4LossTableManager.hh"

#include "G4ComptonScattering.hh"
#include "G4KleinNishinaModel.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"
#include "G4RayleighScattering.hh"
#include "G4LivermorePhotoElectricModel.hh"
#include "G4LivermorePolarizedRayleighModel.hh"
#include "G4PhotoElectricAngularGeneratorPolarized.hh"

#include "G4eMultipleScattering.hh"
#include "G4hMultipleScattering.hh"
#include "G4CoulombScattering.hh"
#include "G4eCoulombScatteringModel.hh"
#include "G4WentzelVIModel.hh"
#include "G4UrbanMscModel.hh"

#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

#include "G4hIonisation.hh"
#include "G4ionIonisation.hh"
#include "G4NuclearStopping.hh"

#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4GenericIon.hh"

#include "G4PhysicsListHelper.hh"
#include "G4BuilderType.hh"
#include "G4EmModelActivator.hh"
#include "G4GammaGeneralProcess.hh"



class ATLEmStandardPhysics : public G4VPhysicsConstructor
{
public:
 explicit ATLEmStandardPhysics(G4int ver = 1, const G4String& name = "")
   : G4VPhysicsConstructor("ATLEmStandard")
 {
   SetVerboseLevel(ver);
   G4EmParameters* param = G4EmParameters::Instance();
   param->SetDefaults();
   param->SetVerbose(ver);
   // param->SetFluctuationType(fUrbanFluctuation);
   SetPhysicsType(bElectromagnetic);
 }

 void ConstructParticle() override;
 void ConstructProcess() override;
};

void ATLEmStandardPhysics::ConstructParticle()
{
 // minimal set of particles for EM physics
 G4EmBuilder::ConstructMinimalEmSet();
}

void ATLEmStandardPhysics::ConstructProcess()
{
  if(verboseLevel > 1) {
    G4cout << "### " << GetPhysicsName() << " Construct Processes " << G4endl;
  }
  G4EmBuilder::PrepareEMPhysics();

  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();
  G4EmParameters* param = G4EmParameters::Instance();

  // processes used by several particles
  G4hMultipleScattering* hmsc = new G4hMultipleScattering("ionmsc");

  // nuclear stopping is enabled if th eenergy limit above zero
  G4double nielEnergyLimit = param->MaxNIELEnergy();
  G4NuclearStopping* pnuc = nullptr;
  if(nielEnergyLimit > 0.0) {
    pnuc = new G4NuclearStopping();
    pnuc->SetMaxKinEnergy(nielEnergyLimit);
  }

  // high energy limit for e+- scattering models and bremsstrahlung
  G4double highEnergyLimit = param->MscEnergyLimit();

  // Add gamma EM Processes
  G4ParticleDefinition* particle = G4Gamma::Gamma();

  // Photoelectric
  G4PhotoElectricEffect* pe = new G4PhotoElectricEffect();
  G4VEmModel* peModel = new G4LivermorePhotoElectricModel();
  pe->SetEmModel(peModel);

  // Compton scattering
  G4ComptonScattering* cs = new G4ComptonScattering;

  // default Rayleigh scattering is Livermore
  G4RayleighScattering* rl = new G4RayleighScattering();

    ph->RegisterProcess(pe, particle);
    ph->RegisterProcess(cs, particle);
    ph->RegisterProcess(new G4GammaConversion(), particle);
    ph->RegisterProcess(rl, particle);

  // e-
  particle = G4Electron::Electron();

//  G4eMultipleScattering* msc = new G4eMultipleScattering;
//  G4UrbanMscModel* msc1 = new G4UrbanMscModel();
//  //G4WentzelVIModel* msc2 = new G4WentzelVIModel();
//  msc1->SetHighEnergyLimit(highEnergyLimit);
//  //msc2->SetLowEnergyLimit(highEnergyLimit);
//  msc->SetEmModel(msc1);
//  //msc->SetEmModel(msc2);
//
  //G4eCoulombScatteringModel* ssm = new G4eCoulombScatteringModel();
  //G4CoulombScattering* ss = new G4CoulombScattering();
  //ss->SetEmModel(ssm);
  //ss->SetMinKinEnergy(highEnergyLimit);
  //ssm->SetLowEnergyLimit(highEnergyLimit);
  //ssm->SetActivationLowEnergyLimit(highEnergyLimit);

//  ph->RegisterProcess(msc, particle);
  ph->RegisterProcess(new G4eIonisation(), particle);
  ph->RegisterProcess(new G4eBremsstrahlung(), particle);
  //ph->RegisterProcess(ss, particle);

  // e+
  particle = G4Positron::Positron();

//  msc = new G4eMultipleScattering;
//  msc1 = new G4UrbanMscModel();
//  //msc2 = new G4WentzelVIModel();
//  msc1->SetHighEnergyLimit(highEnergyLimit);
//  //msc2->SetLowEnergyLimit(highEnergyLimit);
//  msc->SetEmModel(msc1);
//  //msc->SetEmModel(msc2);
//
  //ssm = new G4eCoulombScatteringModel();
  //ss = new G4CoulombScattering();
  //ss->SetEmModel(ssm);
  //ss->SetMinKinEnergy(highEnergyLimit);
  //ssm->SetLowEnergyLimit(highEnergyLimit);
  //ssm->SetActivationLowEnergyLimit(highEnergyLimit);

  //ph->RegisterProcess(msc, particle);
  ph->RegisterProcess(new G4eIonisation(), particle);
  ph->RegisterProcess(new G4eBremsstrahlung(), particle);
  ph->RegisterProcess(new G4eplusAnnihilation(), particle);
  //ph->RegisterProcess(ss, particle);

  // generic ion
  particle = G4GenericIon::GenericIon();
  G4ionIonisation* ionIoni = new G4ionIonisation();
  ph->RegisterProcess(hmsc, particle);
  ph->RegisterProcess(ionIoni, particle);
  if(nullptr != pnuc) { ph->RegisterProcess(pnuc, particle); }

  // muons, hadrons ions
  G4EmBuilder::ConstructCharged(hmsc, pnuc);

  // extra configuration
  G4EmModelActivator mact(GetPhysicsName());
}


ATLFTFP_BERT::ATLFTFP_BERT(G4int ver)
{
  if(ver > 0) {
    G4cout << "<<< Geant4 Physics List simulation engine: FTFP_BERT"<<G4endl;
    G4cout <<G4endl;
  }
  defaultCutValue = 0.7*CLHEP::mm;
  SetVerboseLevel(ver);

 // EM Physics
  RegisterPhysics( new ATLEmStandardPhysics(ver));

  // Synchroton Radiation & GN Physics
  RegisterPhysics( new G4EmExtraPhysics(ver) );

  // Decays
  RegisterPhysics( new G4DecayPhysics(ver) );

   // Hadron Elastic scattering
  RegisterPhysics( new G4HadronElasticPhysics(ver) );

   // Hadron Physics
  RegisterPhysics(  new G4HadronPhysicsFTFP_BERT(ver));

  // Stopping Physics
  RegisterPhysics( new G4StoppingPhysics(ver) );

  // Ion Physics
  RegisterPhysics( new G4IonPhysics(ver));

  // Neutron tracking cut
  RegisterPhysics( new G4NeutronTrackingCut(ver));
}

