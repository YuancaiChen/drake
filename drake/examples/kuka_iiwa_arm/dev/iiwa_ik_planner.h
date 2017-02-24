#pragma once

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "drake/common/drake_copyable.h"
#include "drake/common/eigen_types.h"
#include "drake/common/trajectories/piecewise_polynomial_trajectory.h"
#include "drake/multibody/rigid_body_ik.h"

namespace drake {
namespace examples {
namespace kuka_iiwa_arm {

/**
 * A wrapper class around the IK planner. This class improves IK's usability by
 * handling constraint relaxing and multiple initial guesses internally.
 */
class IiwaIkPlanner {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(IiwaIkPlanner);

  /**
   * Cartesian waypoint. Input to the IK solver.
   */
  struct IkCartesianWaypoint {
    /// Desired end effector pose in the world frame.
    Isometry3<double> pose;
    /// Bounding box for the end effector in the world frame.
    Vector3<double> pos_tol;
    /// Max angle difference (in radians) between solved end effector's
    /// orientation and the desired.
    double rot_tol;
    /// Signals if orientation constraint is enabled.
    bool constrain_orientation;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /// Default constructor.
    IkCartesianWaypoint() {
      pose.setIdentity();
      constrain_orientation = false;
      pos_tol = Vector3<double>(0.005, 0.005, 0.005);
      rot_tol = 0.05;
    }
  };

  /**
   * Returns a linear PiecewisePolynomialTrajectory from @p times and @p ik_res.
   */
  static std::unique_ptr<PiecewisePolynomialTrajectory>
  GenerateFirstOrderHoldTrajectory(const std::vector<double>& times,
                                   const IKResults& ik_res);

  /**
   * Constructor. Instantiates an internal RigidBodyTree from @p model_path.
   * @param model_path Path to the model file.
   * @param end_effector_link_name Link name of the end effector.
   * @param base_to_world X_WB, transformation from robot's base to the world
   * frame.
   * @param random_seed Seed for the random number generator used to generate
   * random initial guesses.
   */
  IiwaIkPlanner(const std::string& model_path,
                const std::string& end_effector_link_name,
                const Isometry3<double>& base_to_world, int random_seed = 1234);

  /**
   * Constructor. Instantiates an internal RigidBodyTree from @p model_path.
   * @param model_path Path to the model file.
   * @param end_effector_link_name Link name of the end effector.
   * @param base_to_world X_WB, transformation from robot's base to the world
   * frame.
   * @param random_seed Seed for the random number generator used to generate
   * random initial guesses.
   */
  IiwaIkPlanner(const std::string& model_path,
                const std::string& end_effector_link_name,
                std::shared_ptr<RigidBodyFrame<double>> base_to_world,
                int random_seed = 1234);

  /**
   * Sets end effector to @p end_effector_body.
   */
  void SetEndEffector(const RigidBody<double>& end_effector_body) {
    end_effector_body_idx_ = end_effector_body.get_body_index();
  }

  /**
   * Sets end effector to @p link_name.
   */
  void SetEndEffector(const std::string& link_name) {
    end_effector_body_idx_ = robot_->FindBodyIndex(link_name);
  }

  /**
   * Returns constant reference to the robot model.
   */
  const RigidBodyTree<double>& get_robot() const { return *robot_; }

  /**
   * Generates IK solutions for each waypoint sequentially. For waypoint wp_i,
   * the IK tries to solve q_i that satisfies the end effector constraints in
   * wp_i and minimizes the squared difference to q_{i-1}, where q_{i-1} is the
   * solution to the previous wp_{i-1}. q_{i-1} = @p q_current when i = 0. This
   * function internally does constraint relaxing and initial condition
   * guessing if necessary.
   *
   * Note that @p q_current is inserted at the beginning of @p ik_res.
   *
   * @param waypoints A sequence of desired waypoints.
   * @param q_current The initial generalized position.
   * @param[out] ik_res Results.
   * @return True if solved successfully.
   */
  bool PlanSequentialTrajectory(
      const std::vector<IkCartesianWaypoint>& waypoints,
      const VectorX<double>& q_current, IKResults* ik_res);

 private:
  bool SolveIk(const IkCartesianWaypoint& waypoint, const VectorX<double>& q0,
               const VectorX<double>& q_nom,
               const Vector3<double>& position_tol, double rot_tolerance,
               VectorX<double>* ik_res,
               std::vector<int>* info,
               std::vector<std::string>* infeasible_constraints);

  std::default_random_engine rand_generator_;
  std::unique_ptr<RigidBodyTree<double>> robot_{nullptr};
  int end_effector_body_idx_;
};

}  // namespace kuka_iiwa_arm
}  // namespace examples
}  // namespace drake
