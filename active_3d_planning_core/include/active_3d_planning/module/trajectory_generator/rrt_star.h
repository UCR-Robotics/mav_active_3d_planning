#ifndef ACTIVE_3D_PLANNING_CORE_TRAJECTORY_GENERATORS_RRT_STAR_H
#define ACTIVE_3D_PLANNING_CORE_TRAJECTORY_GENERATORS_RRT_STAR_H

#include "active_3d_planning/module/trajectory_evaluator.h"
#include "active_3d_planning/module/trajectory_generator/rrt.h"

namespace active_3d_planning {
namespace trajectory_evaluator {
class RRTStarEvaluatorAdapter;
}
namespace generator_updater {
class SimpleRRTStarCollisionUpdater;
}
namespace trajectory_generator {

// Extension of the RRT class. Use with its trajectory evaluator adapter below
// to find the gain, cost and value for new segments and tries rewiring the best
// segments together. RRTStar requires trajectory evaluation based only on the
// last point, since it evaluates and connects points (and not trajectories).
class RRTStar : public RRT {
public:
  RRTStar(PlannerI &planner);
  // override virtual functions
  virtual bool
  expandSegment(TrajectorySegment *target,
                std::vector<TrajectorySegment *> *new_segments) override;

  bool rewireRoot(TrajectorySegment *root, int *next_segment);

  bool rewireIntermediate(TrajectorySegment *root);

  virtual void setupFromParamMap(Module::ParamMap *param_map) override;
  virtual bool checkParamsValid(std::string *error_message) override;

protected:
  friend trajectory_evaluator::RRTStarEvaluatorAdapter;
  friend generator_updater::SimpleRRTStarCollisionUpdater;

  static ModuleFactoryRegistry::Registration<RRTStar> registration;

  // parameters
  bool p_rewire_root_; // if true try rewiring the children of the current
                       // segment (to keep them alive)
  bool p_rewire_intermediate_; // If true try rewiring existing segments to new
                               // candidates if this improves their value
  bool p_rewire_update_;       // If true try rewiring segments during after the
                               // update step
  bool p_update_subsequent_;   // True: update the subtrees of rewired segments
  bool p_reinsert_root_; // True: reinsert the root node at next execution to
                         // guarantee rewireability
  double p_max_rewire_range_;  // distance [m] within which rewiring is possible
  double p_max_density_range_; // only add points if theres no point closer than
                               // this [m], 0.0 to ignore
  int p_n_neighbors_;          // How many knns to consider for rewiring

  // variables
  bool tree_is_reset_; // Force reset of kdtree if requestNext is called twice
                       // (dangling pointers)

  // methods
  bool findNearbyCandidates(const Eigen::Vector3d &target_point,
                            std::vector<TrajectorySegment *> *result);

  bool rewireToBestParent(TrajectorySegment *segment,
                          const std::vector<TrajectorySegment *> &candidates,
                          bool force_rewire = false);

  bool rewireRootSingle(TrajectorySegment *segment,
                        TrajectorySegment *new_root);
};

} // namespace trajectory_generator

namespace trajectory_evaluator {

// RRTStar Evaluator adapter, its following evaluator is used for all regular
// evaluation duties.
class RRTStarEvaluatorAdapter : public TrajectoryEvaluator {
public:
  RRTStarEvaluatorAdapter(PlannerI &planner);
  // Override virtual functions
  bool computeGain(TrajectorySegment *traj_in) override;

  bool computeCost(TrajectorySegment *traj_in) override;

  bool computeValue(TrajectorySegment *traj_in) override;

  int selectNextBest(TrajectorySegment *traj_in) override;

  bool updateSegments(TrajectorySegment *root) override;

  void visualizeTrajectoryValue(VisualizerI& visualizer,
                                const TrajectorySegment &trajectory);

  void setupFromParamMap(Module::ParamMap *param_map) override;

protected:
  static ModuleFactoryRegistry::Registration<RRTStarEvaluatorAdapter>
      registration;

  // members
  std::unique_ptr<TrajectoryEvaluator> following_evaluator_;
  trajectory_generator::RRTStar *generator_;
};

} // namespace trajectory_evaluator
} // namespace active_3d_planning

#endif // ACTIVE_3D_PLANNING_CORE_TRAJECTORY_GENERATORS_RRT_STAR_H
