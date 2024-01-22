[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_CinematicHelperComponentClass : ScriptComponentClass
{
}

class SCR_CinematicHelperComponent : ScriptComponent
{
	[Attribute(desc: "A UI window will be created, showing the distances between them & the currently active camera (for DOF?)")]
	ref array<string> m_sEntitiesForDistanceMeasure = {};
	
	vector m_ownerTransform[4];
	CameraManager m_CameraManager;
	CameraBase m_ActiveCamera;
	
	bool m_useStaticTransformSnapshot = false;
	
	vector m_mCameraTransformSnapshot[4];
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		BaseWorld world = GetGame().GetWorld();
		
		if (!GetGame().InPlayMode())
			return;
		
		
		
		Math3D.MatrixIdentity4(m_ownerTransform);
		owner.GetWorldTransform(m_ownerTransform);
		Math3D.MatrixIdentity4(m_mCameraTransformSnapshot);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if(!m_CameraManager)
			m_CameraManager = CameraManager.Cast(GetGame().GetCameraManager());
		
		if(!m_ActiveCamera)
			m_ActiveCamera = m_CameraManager.CurrentCamera();
		
		if(m_ActiveCamera != m_CameraManager.CurrentCamera())
			m_ActiveCamera = m_CameraManager.CurrentCamera();
		
		if(!m_ActiveCamera)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_ActiveCamera)
			return;
		
		DoSexyDebugWindow();
	}
	//------------------------------------------------------------------------------------------------
	void SetCameraTransformSnapshot()
	{
		m_useStaticTransformSnapshot = true;
		m_ActiveCamera.GetWorldTransform(m_mCameraTransformSnapshot);
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_FREE, 1);
	}

	//------------------------------------------------------------------------------------------------
	void GetOffsets(out vector posOffset, out vector rotOffset)
	{
		
		vector matActiveCam[4];
		m_ActiveCamera.GetWorldTransform(matActiveCam);
		vector rotActiveCam[3];
		for (int i = 0; i < 3; i++)
		{rotActiveCam[i] = matActiveCam[i]}
		
		vector matHelperCam[4];
		vector rotHelperCam[3];
		if (!m_useStaticTransformSnapshot)
			matHelperCam = m_ownerTransform;
		else
			matHelperCam = m_mCameraTransformSnapshot;
		
		for (int i = 0; i < 3; i++)
		{rotHelperCam[i] = matHelperCam[i]}
		
		//PositionOffset
		if (!m_useStaticTransformSnapshot)
			posOffset = matHelperCam[3] - matActiveCam[3];
		else
			posOffset = matActiveCam[3] - matHelperCam[3];
		
		//Now dealng with fucking annoying rotation brotha
		// Convert matrices to quaternions
		float quatActiveCam[4], quatHelperCam[4];
		Math3D.MatrixToQuat(matActiveCam, quatActiveCam);
		Math3D.MatrixToQuat(matHelperCam, quatHelperCam);
		
		// Invert the first quaternion and multiply with the second
		float quatCamInverse[4];
		float quatDifference[4];
		if(!m_useStaticTransformSnapshot)
		{
			Math3D.QuatInverse(quatCamInverse, quatActiveCam);
			Math3D.QuatMultiply(quatDifference, quatCamInverse, quatHelperCam);
		}
		else
		{
			Math3D.QuatInverse(quatCamInverse, quatHelperCam);
			Math3D.QuatMultiply(quatDifference, quatCamInverse, quatActiveCam);
		}
		
		// Convert the difference quaternion to Euler angles (yaw, pitch, roll NOTE !!! our game has different indexes)
		vector angleDifference = Math3D.QuatToAngles(quatDifference);
		rotOffset[0] = angleDifference[1];
		rotOffset[1] = angleDifference[0];
		rotOffset[2] = angleDifference[2];
	}
	
	//------------------------------------------------------------------------------------------------
	void ActiveCamPrintDebug()
	{
		if (!m_ActiveCamera)
			return;
		
		vector mat[4];
		m_ActiveCamera.GetWorldTransform(mat);
		vector pos = mat[3];
		vector angles = m_ActiveCamera.GetAngles();
		PrintFormat("_AS ActiveCam Pos: %1, Angles: %2", pos, angles, LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	void DoSexyDebugWindow()
	{
		if(!m_ActiveCamera)
			return;
		
		vector matActiveCam[4];
		m_ActiveCamera.GetWorldTransform(matActiveCam);
		vector rotMat[3];
		Math3D.MatrixIdentity3(rotMat);
		
		for (int i = 0; i < 3; i++)
		{
			rotMat[i] = matActiveCam[i];
			//DbgUI.Text("HAHA" + rotMat[i]);
		}
				
		string rotMatString = string.Format("[ %1, %2, %3 ]", rotMat[0], rotMat[1], rotMat[2]);
		
		vector posOffset;
		vector rotOffset;
		GetOffsets(posOffset, rotOffset);
		
		DbgUI.Begin("ActiveCam Info");
//		DbgUI.Text("Active Cam Pos " + matActiveCam[3]);
//		if (DbgUI.Button("Copy Active Cam Pos"))
//			{
//				System.ExportToClipboard(matActiveCam[3].ToString());
//			}
//				
//		DbgUI.Text("Active Cam RotMatrix " + rotMatString);
//		if (DbgUI.Button("Copy Active Cam RotMatrix"))
//			{
//				System.ExportToClipboard(rotMatString);
//			}
//		
//		DbgUI.Text("Active Cam RotMatToAngles " + Math3D.MatrixToAngles(rotMat));
//		if (DbgUI.Button("Copy Active Cam RotMatToAngles"))
//			{
//				System.ExportToClipboard(Math3D.MatrixToAngles(rotMat).ToString());
//			}
		DbgUI.Text("PosOffset " + posOffset);
		DbgUI.Text("RotOffset " + rotOffset);
		if (DbgUI.Button("Copy PosOffset + RotOffset"))
			{
				System.ExportToClipboard("posOffset = \""
										+ posOffset[0].ToString() + " "
										+ posOffset[1].ToString() + " "
										+ posOffset[2].ToString() + "\"; "
										+ "rotOffset = \""
										+ rotOffset[0].ToString() + " "
										+ rotOffset[1].ToString() + " "
										+ rotOffset[2].ToString() + "\";"
										);
			}
		DbgUI.End();

	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		// remove if unused
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FIXEDFRAME | EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		// remove if unused
	}
	/*
	override void EOnActivate(IEntity owner)
	{
		super.EOnActivate(owner);
		SetEventMask(owner, EntityEvent.FRAME);
	}
 
	override void EOnDeactivate(IEntity owner)
	{
		super.EOnDeactivate(owner);
		ClearEventMask(owner, EntityEvent.FRAME);
	}
	*/
}
/*
				
			DbgUI.Text("DbgUI Test");
	
			string name = "";
			DbgUI.InputText("name", name);
	
			if (DbgUI.Button("Print name"))
			{
				Print(name);
			}
	
			DbgUI.List("test list", m_DbgListSelection, m_DbgOptions);
	
			DbgUI.Text("Choice = " + m_DbgListSelection.ToString());
	
			DbgUI.Spacer(10);
			DbgUI.SliderFloat("slider", m_DbgSliderValue, 0, 100);
			DbgUI.Text("Slider value = " + m_DbgSliderValue);
		
		DbgUI.End();
		}

	void BigDebug()
	{
				Print("BIG DEBUG");
				// 1. Matrix Multiplication
				vector matA[3];
				matA[0] = Vector(1, 0, 0); matA[1] = Vector(0, 1, 0); matA[2] = Vector(0, 0, 1);
				vector matB[3];
				matB[0] = Vector(1, 2, 3); matB[1] = Vector(4, 5, 6); matB[2] = Vector(7, 8, 9);
				vector matResult[3];
				Math3D.MatrixMultiply3(matA, matB, matResult);
				Print("Matrix Multiplication Result: " + matResult[0].ToString() + ", " + matResult[1].ToString() + ", " + matResult[2].ToString());
				
				// 2. Matrix Inversion
				vector mat[3];
				mat[0] = Vector(1, 2, 3); mat[1] = Vector(0, 1, 4); mat[2] = Vector(5, 6, 0);
				vector matInv[3];
				Math3D.MatrixGetInverse3(mat, matInv);
				Print("Matrix Inversion Result: " + matInv[0].ToString() + ", " + matInv[1].ToString() + ", " + matInv[2].ToString());
				
				// 3. Quaternion Conversion
				float quat[4];
				Math3D.MatrixToQuat(mat, quat);
				Print("Quaternion Conversion Result: " + quat[0].ToString() + ", " + quat[1].ToString() + ", " + quat[2].ToString() + ", " + quat[3].ToString());
				
				// 4. Quaternion Inversion
				float quatInv[4];
				Math3D.QuatInverse(quatInv, quat);
				Print("Quaternion Inversion Result: " + quatInv[0].ToString() + ", " + quatInv[1].ToString() + ", " + quatInv[2].ToString() + ", " + quatInv[3].ToString());
				
				// 5. Quaternion Multiplication
				float quatA[4] = {0.7, 0.1, 0.2, 0.6};
				float quatB[4] = {0.3, 0.4, 0.5, 0.6};
				float quatMult[4];
				Math3D.QuatMultiply(quatMult, quatA, quatB);
				Print("Quaternion Multiplication Result: " + quatMult[0].ToString() + ", " + quatMult[1].ToString() + ", " + quatMult[2].ToString() + ", " + quatMult[3].ToString());
				
				// 6. Quaternion to Matrix Conversion
				vector matFromQuat[3];
				Math3D.QuatToMatrix(quat, matFromQuat);
				Print("Matrix from Quaternion Result: " + matFromQuat[0].ToString() + ", " + matFromQuat[1].ToString() + ", " + matFromQuat[2].ToString());
				
				// 7. Matrix to Euler Angles Conversion
				vector euler = Math3D.MatrixToAngles(mat);
				Print("Matrix to Euler Angles Conversion Result: "+ euler[0].ToString() + ", " + euler[1].ToString() + ", " + euler[2].ToString());
				
				// 8. Vector Normalization
				vector vec = Vector(3, 4, 0);
				vec.Normalize();
				Print("Vector Normalization Result: " + vec);
		
				
				// 9. Vector to Quaternion Conversion
				vector angles = Vector(45, 30, 60);
				float quatFromAngles[4];
				angles.QuatFromAngles(quatFromAngles);
				Print("Quaternion from Euler Angles Result: "+ quatFromAngles[0].ToString() + ", " + quatFromAngles[1].ToString() + ", " + quatFromAngles[2].ToString() + ", " + quatFromAngles[3].ToString());
		
	}
*/
