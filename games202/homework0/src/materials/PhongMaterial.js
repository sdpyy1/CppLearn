class PhongMaterial extends Material {
    /**
    * 创建一个 PhongMaterial 的实例。
    * @param {vec3f} color 材质的颜色
    * @param {Texture} colorMap 材质的纹理对象
    * @param {vec3f} specular 材质的镜面反射系数
    * @param {float} intensity 光照强度
    * @memberof PhongMaterial
    */
    constructor(color, colorMap, specular, intensity) {
        let textureSample = 0; // 初始化纹理采样标志
        if (colorMap != null) {
            textureSample = 1; 
            // 使用super调用父类构造函数
            super({
                'uTextureSample': { type: '1i', value: textureSample },
                'uSampler': { type: 'texture', value: colorMap },
                'uKd': { type: '3fv', value: color },
                'uKs': { type: '3fv', value: specular },
                'uLightIntensity': { type: '1f', value: intensity }
            }, [], PhongVertexShader, PhongFragmentShader);
        } else {
            // 如果不存在纹理，仅传递必要的统一变量
            super({
                'uTextureSample': { type: '1i', value: textureSample },
                'uKd': { type: '3fv', value: color },
                'uKs': { type: '3fv', value: specular },
                'uLightIntensity': { type: '1f', value: intensity }
            }, [], PhongVertexShader, PhongFragmentShader);
        }
    }
}