/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2025, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/

/** @file scene.h
 *  @brief Defines the data structures in which the imported scene is returned.
 */
#pragma once
#ifndef AI_SCENE_H_INC
#define AI_SCENE_H_INC

#ifdef __GNUC__
#   pragma GCC system_header
#endif

#include <assimp/types.h>
#include <assimp/texture.h>
#include <assimp/mesh.h>
#include <assimp/light.h>
#include <assimp/camera.h>
#include <assimp/material.h>
#include <assimp/anim.h>
#include <assimp/metadata.h>

#ifdef __cplusplus
#  include <cstdlib>
extern "C" {
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif

// -------------------------------------------------------------------------------
/**
 * A node in the imported hierarchy.
 *
 * Each node has name, a parent node (except for the root node),
 * a transformation relative to its parent and possibly several child nodes.
 * Simple file formats don't support hierarchical structures - for these formats
 * the imported scene does consist of only a single root node without children.
 */
// -------------------------------------------------------------------------------
struct ASSIMP_API aiNode {
    /** The name of the node.
     *
     * The name might be empty (length of zero) but all nodes which
     * need to be referenced by either bones or animations are named.
     * Multiple nodes may have the same name, except for nodes which are referenced
     * by bones (see #aiBone and #aiMesh::mBones). Their names *must* be unique.
     *
     * Cameras and lights reference a specific node by name - if there
     * are multiple nodes with this name, they are assigned to each of them.
     * <br>
     * There are no limitations with regard to the characters contained in
     * the name string as it is usually taken directly from the source file.
     *
     * Implementations should be able to handle tokens such as whitespace, tabs,
     * line feeds, quotation marks, ampersands etc.
     *
     * Sometimes assimp introduces new nodes not present in the source file
     * into the hierarchy (usually out of necessity because sometimes the
     * source hierarchy format is simply not compatible). Their names are
     * surrounded by @verbatim <> @endverbatim e.g.
     *  @verbatim<DummyRootNode> @endverbatim.
     */
    C_STRUCT aiString mName;

    /** The transformation relative to the node's parent. */
    C_STRUCT aiMatrix4x4 mTransformation;

    /** Parent node. nullptr if this node is the root node. */
    C_STRUCT aiNode* mParent;

    /** The number of child nodes of this node. */
    unsigned int mNumChildren;

    /** The child nodes of this node. nullptr if mNumChildren is 0. */
    C_STRUCT aiNode** mChildren;

    /** The number of meshes of this node. */
    unsigned int mNumMeshes;

    /** The meshes of this node. Each entry is an index into the
      * mesh list of the #aiScene.
      */
    unsigned int* mMeshes;

    /** Metadata associated with this node or nullptr if there is no metadata.
      *  Whether any metadata is generated depends on the source file format. See the
      * @link importer_notes @endlink page for more information on every source file
      * format. Importers that don't document any metadata don't write any.
      */
    C_STRUCT aiMetadata* mMetaData;

#ifdef __cplusplus
    /** Constructor */
    aiNode();

    /** Construction from a specific name */
    explicit aiNode(const std::string& name);

    /** Destructor */
    ~aiNode();

    /**
     *  @brief Searches for a node with a specific name, beginning at this
     *  nodes. Normally you will call this method on the root node
     *  of the scene.
     *
     *  @param name Name to search for
     *  @return nullptr or a valid Node if the search was successful.
     */
    inline const aiNode* FindNode(const aiString& name) const {
        return FindNode(name.data);
    }

    inline aiNode* FindNode(const aiString& name) {
        return FindNode(name.data);
    }

    /**
     * @brief Will search for a node described by its name.
     * @param[in] name  The name for the node to look for.
     * @return Pointer showing to the node or nullptr if not found.
     */
    const aiNode* FindNode(const char* name) const;
    aiNode* FindNode(const char* name);

    // ------------------------------------------------------------------------------------------------
    // Helper to find the node associated with a bone in the scene
    const aiNode *findBoneNode(const aiBone *bone) const {
        if (bone == nullptr) {
            return nullptr;
        }

        if (mName == bone->mName) {
            return this;
        }

        for (unsigned int i = 0; i < mNumChildren; ++i) {
            aiNode *aChild = mChildren[i];
            if (aChild == nullptr) {
                continue;
            }

            const aiNode *foundFromChild = nullptr;
            foundFromChild = aChild->findBoneNode(bone);
            if (foundFromChild) {
                return foundFromChild;
            }
        }

        return nullptr;
    }

    /**
     * @brief   Will add new children.
     * @param   numChildren  Number of children to add.
     * @param   children     The array with pointers showing to the children.
     */
    void addChildren(unsigned int numChildren, aiNode **children);
#endif // __cplusplus
};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

// -------------------------------------------------------------------------------
/**
 * Specifies that the scene data structure that was imported is not complete.
 * This flag bypasses some internal validations and allows the import
 * of animation skeletons, material libraries or camera animation paths
 * using Assimp. Most applications won't support such data.
 */
#define AI_SCENE_FLAGS_INCOMPLETE   0x1

/**
 * This flag is set by the validation postprocess-step (aiPostProcess_ValidateDS)
 * if the validation is successful. In a validated scene you can be sure that
 * any cross references in the data structure (e.g. vertex indices) are valid.
 */
#define AI_SCENE_FLAGS_VALIDATED    0x2

/**
 * This flag is set by the validation postprocess-step (aiPostProcess_ValidateDS)
 * if the validation is successful but some issues have been found.
 * This can for example mean that a texture that does not exist is referenced
 * by a material or that the bone weights for a vertex don't sum to 1.0 ... .
 * In most cases you should still be able to bind the import. This flag could
 * be useful for applications which don't capture Assimp's log output.
 */
#define AI_SCENE_FLAGS_VALIDATION_WARNING   0x4

/**
 * This flag is currently only set by the aiProcess_JoinIdenticalVertices step.
 * It indicates that the vertices of the output meshes aren't in the internal
 * verbose format anymore. In the verbose format all vertices are unique,
 * no vertex is ever referenced by more than one face.
 */
#define AI_SCENE_FLAGS_NON_VERBOSE_FORMAT   0x8

 /**
 * Denotes pure height-map terrain data. Pure terrains usually consist of quads,
 * sometimes triangles, in a regular grid. The x,y coordinates of all vertex
 * positions refer to the x,y coordinates on the terrain height map, the z-axis
 * stores the elevation at a specific point.
 *
 * TER (Terragen) and HMP (3D Game Studio) are height map formats.
 * @note Assimp is probably not the best choice for loading *huge* terrains -
 * fully triangulated data takes extremely much free store and should be avoided
 * as long as possible (typically you'll do the triangulation when you actually
 * need to render it).
 */
#define AI_SCENE_FLAGS_TERRAIN 0x10

 /**
 * Specifies that the scene data can be shared between structures. For example:
 * one vertex in few faces. \ref AI_SCENE_FLAGS_NON_VERBOSE_FORMAT can not be
 * used for this because \ref AI_SCENE_FLAGS_NON_VERBOSE_FORMAT has internal
 * meaning about postprocessing steps.
 */
#define AI_SCENE_FLAGS_ALLOW_SHARED			0x20

// -------------------------------------------------------------------------------
/** The root structure of the imported data.
 *
 *  Everything that was imported from the given file can be accessed from here.
 *  Objects of this class are generally maintained and owned by Assimp, not
 *  by the caller. You shouldn't want to instance it, nor should you ever try to
 *  delete a given scene on your own.
 */
// -------------------------------------------------------------------------------
struct ASSIMP_API aiScene {
    /** Any combination of the AI_SCENE_FLAGS_XXX flags. By default
    * this value is 0, no flags are set. Most applications will
    * want to reject all scenes with the AI_SCENE_FLAGS_INCOMPLETE
    * bit set.
    */
    unsigned int mFlags;

    /** The root node of the hierarchy.
    *
    * There will always be at least the root node if the import
    * was successful (and no special flags have been set).
    * Presence of further nodes depends on the format and content
    * of the imported file.
    */
    C_STRUCT aiNode* mRootNode;

    /** The number of meshes in the scene. */
    unsigned int mNumMeshes;

    /** The array of meshes.
    *
    * Use the indices given in the aiNode structure to access
    * this array. The array is mNumMeshes in size. If the
    * AI_SCENE_FLAGS_INCOMPLETE flag is not set there will always
    * be at least ONE material.
    */
    C_STRUCT aiMesh** mMeshes;

    /** The number of materials in the scene. */
    unsigned int mNumMaterials;

    /** The array of materials.
    *
    * Use the index given in each aiMesh structure to access this
    * array. The array is mNumMaterials in size. If the
    * AI_SCENE_FLAGS_INCOMPLETE flag is not set there will always
    * be at least ONE material.
    */
    C_STRUCT aiMaterial** mMaterials;

    /** The number of animations in the scene. */
    unsigned int mNumAnimations;

    /** The array of animations.
    *
    * All animations imported from the given file are listed here.
    * The array is mNumAnimations in size.
    */
    C_STRUCT aiAnimation** mAnimations;

    /** The number of textures embedded into the file */
    unsigned int mNumTextures;

    /** The array of embedded textures.
    *
    * Not many file formats embed their textures into the file.
    * An example is Quake's MDL format (which is also used by
    * some GameStudio versions)
    */
    C_STRUCT aiTexture** mTextures;

    /** The number of light sources in the scene. Light sources
    * are fully optional, in most cases this attribute will be 0
        */
    unsigned int mNumLights;

    /** The array of light sources.
    *
    * All light sources imported from the given file are
    * listed here. The array is mNumLights in size.
    */
    C_STRUCT aiLight** mLights;

    /** The number of cameras in the scene. Cameras
    * are fully optional, in most cases this attribute will be 0
        */
    unsigned int mNumCameras;

    /** The array of cameras.
    *
    * All cameras imported from the given file are listed here.
    * The array is mNumCameras in size. The first camera in the
    * array (if existing) is the default camera view into
    * the scene.
    */
    C_STRUCT aiCamera** mCameras;

    /**
     *  @brief  The global metadata assigned to the scene itself.
     *
     *  This data contains global metadata which belongs to the scene like
     *  unit-conversions, versions, vendors or other model-specific data. This
     *  can be used to store format-specific metadata as well.
     */
    C_STRUCT aiMetadata* mMetaData;

    /** The name of the scene itself.
     */
    C_STRUCT aiString mName;

    /**
     *
     */
    unsigned int mNumSkeletons;

    /**
     *
     */
    C_STRUCT aiSkeleton **mSkeletons;

#ifdef __cplusplus

    //! Default constructor - set everything to 0/nullptr
    aiScene();

    //! Destructor
    ~aiScene();

    //! Check whether the scene contains meshes
    //! Unless no special scene flags are set this will always be true.
    inline bool HasMeshes() const {
        return mMeshes != nullptr && mNumMeshes > 0;
    }

    //! Check whether the scene contains materials
    //! Unless no special scene flags are set this will always be true.
    inline bool HasMaterials() const {
        return mMaterials != nullptr && mNumMaterials > 0;
    }

    //! Check whether the scene contains lights
    inline bool HasLights() const {
        return mLights != nullptr && mNumLights > 0;
    }

    //! Check whether the scene contains textures
    inline bool HasTextures() const {
        return mTextures != nullptr && mNumTextures > 0;
    }

    //! Check whether the scene contains cameras
    inline bool HasCameras() const {
        return mCameras != nullptr && mNumCameras > 0;
    }

    //! Check whether the scene contains animations
    inline bool HasAnimations() const {
        return mAnimations != nullptr && mNumAnimations > 0;
    }

    //! Check whether the scene contains skeletons
    inline bool HasSkeletons() const {
        return mSkeletons != nullptr && mNumSkeletons > 0;
    }

    //! Returns a short filename from a full path
    static const char* GetShortFilename(const char* filename) {
        const char* lastSlash = strrchr(filename, '/');
        const char* lastBackSlash = strrchr(filename, '\\');
        if (lastSlash < lastBackSlash) {
            lastSlash = lastBackSlash;
        }
        const char* shortFilename = lastSlash != nullptr ? lastSlash + 1 : filename;
        return shortFilename;
    }

    //! Returns an embedded texture
    const aiTexture* GetEmbeddedTexture(const char* filename) const {
        return GetEmbeddedTextureAndIndex(filename).first;
    }

    //! Returns an embedded texture and its index
    std::pair<const aiTexture*, int> GetEmbeddedTextureAndIndex(const char* filename) const {
        if (nullptr==filename) {
            return std::make_pair(nullptr, -1);
        }
        // lookup using texture ID (if referenced like: "*1", "*2", etc.)
        if ('*' == *filename) {
            int index = std::atoi(filename + 1);
            if (0 > index || mNumTextures <= static_cast<unsigned>(index)) {
                return std::make_pair(nullptr, -1);
            }
            return std::make_pair(mTextures[index], index);
        }
        // lookup using filename
        const char* shortFilename = GetShortFilename(filename);
        if (nullptr == shortFilename) {
            return std::make_pair(nullptr, -1);
        }

        for (unsigned int i = 0; i < mNumTextures; i++) {
            const char* shortTextureFilename = GetShortFilename(mTextures[i]->mFilename.C_Str());
            if (strcmp(shortTextureFilename, shortFilename) == 0) {
                return std::make_pair(mTextures[i], static_cast<int>(i));
            }
        }
        return std::make_pair(nullptr, -1);
    }

    /**
     * @brief Will try to locate a bone described by its name.
     *
     * @param name  The name to look for.
     * @return The bone as a pointer.
     */
    inline aiBone *findBone(const aiString &name) const {
        for (size_t m = 0; m < mNumMeshes; m++) {
            aiMesh *mesh = mMeshes[m];
            if (mesh == nullptr) {
                continue;
            }

            for (size_t b = 0; b < mesh->mNumBones; b++) {
                aiBone *bone = mesh->mBones[b];
                if (bone == nullptr) {
                    continue;
                }
                if (name == bone->mName) {
                    return bone;
                }
            }
        }
        return nullptr;
    }

#endif // __cplusplus

    /**  Internal data, do not touch */
#ifdef __cplusplus
    void* mPrivate;
#else
    char* mPrivate;
#endif

};

#ifdef __cplusplus
}
#endif //! extern "C"

#endif // AI_SCENE_H_INC
