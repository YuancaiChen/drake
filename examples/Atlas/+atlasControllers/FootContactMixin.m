classdef FootContactMixin 
  properties
    contact_threshold = 0.001;
    using_flat_terrain = false;
    robot
    mex_ptr;
  end

  methods
    function obj = FootContactMixin(r, options)
      obj = applyDefaults(obj, options);
      obj.robot = r;

      terrain = getTerrain(r);
      if isa(terrain,'DRCTerrainMap') 
        terrain_map_ptr = terrain.map_handle.getPointerForMex();
      else
        terrain_map_ptr = 0;
      end
      
      if exist('supportDetectmex','file')~=3
        error('can''t find supportDetectmex.  did you build it?');
      end      
      obj.mex_ptr = SharedDataHandle(supportDetectmex(0,r.getMexModelPtr.ptr,terrain_map_ptr));
    end

    function fc = getFC(obj, x, planned_supp, contact_sensor)
      if ~obj.using_flat_terrain
        height = getTerrainHeight(obj.robot,[0;0]); % get height from DRCFlatTerrainMap
      else
        height = 0;
      end
      active_supports = supportDetectmex(obj.mex_ptr.data,x,planned_supp,contact_sensor,contact_thresh,height,contact_logic_AND);
      fc = [1.0*any(active_supports==obj.robot.foot_body_id.left); 1.0*any(active_supports==obj.robot.foot_body_id.right)];
    end
  end
end