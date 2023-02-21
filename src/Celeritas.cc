#include "Celeritas.hh"

#include <G4Threading.hh>
#include <accel/AlongStepFactory.hh>
#include <celeritas/field/UniformFieldData.hh>
#include <celeritas/global/alongstep/AlongStepGeneralLinearAction.hh>
#include <celeritas/global/alongstep/AlongStepUniformMscAction.hh>
#include <celeritas/io/ImportData.hh>

#include <memory>

using namespace celeritas;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
namespace
{

std::shared_ptr<ExplicitActionInterface const> make_nofield_along_step(
  AlongStepFactoryInput const& input)
{
    CELER_LOG(debug) << "Creating along-step action with linear propagation";
    return celeritas::AlongStepGeneralLinearAction::from_params(input.action_id, *input.material,
      *input.particle, *input.physics, input.imported->em_params.energy_loss_fluct);
}

}  // namespace
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// Global shared setup options
SetupOptions& CelerSetupOptions()
{
  static SetupOptions options = [] {
    // Construct setup options the first time CelerSetupOptions is invoked
    SetupOptions so;

    // Set along-step factory
    so.make_along_step = make_nofield_along_step;

    so.max_num_tracks = 1024;
    so.max_num_events = 1024;
    so.initializer_capacity = 1024 * 128;
    so.secondary_stack_factor = 3.0;
    so.ignore_processes = {"CoulombScat", "muIoni", "muBrems", "muPairProd"};

    // Use Celeritas "hit processor" to call back to Geant4 SDs.
    so.sd.enabled = true;

    // Only call back for nonzero energy depositions: this is currently a
    // global option for all detectors, so if any SDs extract data from tracks
    // with no local energy deposition over the step, it must be set to false.
    so.sd.ignore_zero_deposition = true;

    // Using the pre-step point, reconstruct the G4 touchable handle.
    so.sd.locate_touchable = true;

    // Pre-step time is used
    so.sd.pre.global_time = true;
    return so;
  }();
  return options;
}

// Shared data and GPU setup
SharedParams& CelerSharedParams()
{
  static SharedParams sp;
  return sp;
}

// Thread-local transporter
LocalTransporter& CelerLocalTransporter()
{
  static G4ThreadLocal LocalTransporter lt;
  return lt;
}
