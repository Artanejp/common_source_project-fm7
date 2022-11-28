using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;

public class pyuta_unity : MonoBehaviour
{
    [DllImport("pyuta_unity")]
    private static extern IntPtr Init();
    [DllImport("pyuta_unity")]
    private static extern void Exec();
    [DllImport("pyuta_unity")]
    private static extern void StopEmulation();
    [DllImport("pyuta_unity")]
    private static extern IntPtr CheckResident();
    [DllImport("pyuta_unity")]
    private static extern int GetEmulWidth(IntPtr ptr);
    [DllImport("pyuta_unity")]
    private static extern int GetEmulHeight(IntPtr ptr);
    [DllImport("pyuta_unity")]
    private static extern void SetEmulTexturePtr(IntPtr ptr, IntPtr texture);
    [DllImport("pyuta_unity")]
    private static extern void SetEmulPause(IntPtr ptr, bool fPause);

    [DllImport("pyuta_unity")]
    private static extern IntPtr GetRenderEventFunc();

    private IntPtr _emul = IntPtr.Zero;

    void Start()
    {
        Debug.Log("***Start");

        _emul = CheckResident();

        var tex = new Texture2D(
            GetEmulWidth(_emul),
            GetEmulHeight(_emul),
            TextureFormat.ARGB32,
            false);
        GetComponent<Renderer>().material.mainTexture = tex;

        SetEmulTexturePtr(_emul, tex.GetNativeTexturePtr());

        SetEmulPause(_emul, false);
        Exec();
        StartCoroutine(OnRender());

    }

    void OnDisable()
    {
        Debug.Log("***OnDisable");
        if (_emul == IntPtr.Zero) return;

        Debug.Log("**Disable:Pause On");       // エディタでの停止
        SetEmulPause(_emul, true);
    }

    void OnEnable()
    {
        Debug.Log("***OnEnable");
        if (_emul == IntPtr.Zero) return;

        Debug.Log("**Enable:Pause Off");       // エディタでの再開
        SetEmulPause(_emul, false);
    }

    private void OnDestroy()
    {
        Debug.Log("***OnDestroy");
        StopEmulation();
    }



    IEnumerator OnRender()
	{
		for (;;) {
			yield return new WaitForEndOfFrame();
			GL.IssuePluginEvent(GetRenderEventFunc(), 0);
		}
	}
}
