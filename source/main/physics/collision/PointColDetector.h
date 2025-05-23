/*
    This source file is part of Rigs of Rods
    Copyright 2009 Lefteris Stamatogiannakis

    For more information, see http://www.rigsofrods.org/

    Rigs of Rods is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3, as
    published by the Free Software Foundation.

    Rigs of Rods is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rigs of Rods. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Application.h"

#include <unordered_set>

namespace RoR {

/// @addtogroup Physics
/// @{

/// @addtogroup Collisions
/// @{

class PointColDetector
{
public:

    struct pointid_t // use PointidID_t for indexing
    {
        ActorInstanceID_t actorid = ACTORINSTANCEID_INVALID;
        NodeNum_t nodenum = NODENUM_INVALID;
    };

    std::vector<PointidID_t> hit_list;
    std::unordered_set<ActorInstanceID_t> hit_list_actorset;
    std::vector<pointid_t> hit_pointid_list;

    PointColDetector(ActorPtr actor): m_actor(actor), m_object_list_size(-1) {};

    void UpdateIntraPoint(bool contactables = false);
    void UpdateInterPoint(bool ignorestate = false);
    void query(const Ogre::Vector3& vec1, const Ogre::Vector3& vec2, const Ogre::Vector3& vec3, const float enlargeBB);

private:

    struct refelem_t // use RefelemID_t for indexing
    {
        PointidID_t pidrefid = POINTIDID_INVALID;
        std::array<float, 3> point; // cached node AbsPosition
        void setPoint(const Ogre::Vector3 pos) { point[0] = pos.x; point[1] = pos.y; point[2] = pos.z; }
    };

    struct kdnode_t
    {
        float min;
        int end;
        float max;
        RefelemID_t refid = REFELEMID_INVALID;
        float middle;
        int begin;
    };

    ActorPtr                 m_actor;
    std::vector<ActorInstanceID_t>    m_collision_partners; //!< IntraPoint: always just owning actor; InterPoint: all colliding actors
    std::vector<refelem_t> m_ref_list;
    
    std::vector<kdnode_t>  m_kdtree;
    Ogre::Vector3          m_bbmin = Ogre::Vector3::ZERO;
    Ogre::Vector3          m_bbmax = Ogre::Vector3::ZERO;
    int                    m_object_list_size = 0;

    void queryrec(int kdindex, int axis);
    void build_kdtree_incr(int axis, int index);
    void partintwo(const int start, const int median, const int end, const int axis, float& minex, float& maxex);
    void update_structures_for_contacters(bool ignoreinternal);
    void refresh_node_positions();
};

/// @} // addtogroup Collisions
/// @} // addtogroup Physics

} // namespace RoR
