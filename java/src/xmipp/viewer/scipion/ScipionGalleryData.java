/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package xmipp.viewer.scipion;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import xmipp.ij.commons.Geometry;
import xmipp.jni.EllipseCTF;
import xmipp.jni.MetaData;
import xmipp.utils.Params;
import xmipp.utils.XmippDialog;
import xmipp.viewer.models.ClassInfo;
import xmipp.viewer.models.ColumnInfo;
import xmipp.viewer.models.GalleryData;

/**
 *
 * @author airen
 */
public class ScipionGalleryData extends GalleryData {
    
    public ScipionGalleryData(ScipionGalleryJFrame window, String fn, Params parameters) {
        this(window, parameters, new ScipionMetaData(fn));
    }

    public ScipionGalleryData(ScipionGalleryJFrame window, Params parameters, ScipionMetaData md) {
        super(window, parameters, md);

        mdBlocks = md.getBlocks();
        selectedBlock = mdBlocks[0];

    }

    public void setFileName(String file) {
        if (file.contains("@")) {
            int sep = file.lastIndexOf("@");
            selectedBlock = file.substring(0, sep);
            filename = file.substring(sep + 1);
        }
        filename = file;

    }

    
    @Override
    public ColumnInfo initColumnInfo(int label)
    {
        return ((ScipionMetaData)md).getColumnInfo(label);
    }

    public String getValueFromLabel(int index, int label) {
        return ((ScipionMetaData) md).getValueFromLabel(index, label);
    }

    

    

    public boolean isColumnFormat() {
        return true;
    }

    /**
     * Create a metadata just with selected items
     */
    @Override
    public ScipionMetaData getSelectionMd() {
        return null;//metadata operations are not used in scipion
    }

    /**
     * Get all the images assigned to all selected classes
     */
    public MetaData getClassesImages() {
        return null;
    }

    

    /**
     * Get the metadata with assigned images to this classes
     */
    public MetaData getClassImages(int index) {
        try {
            long id = ids[index];
            ScipionMetaData childmd = ((ScipionMetaData) md).getEMObject(id).childmd;
            return childmd;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public String getLabel(long objId, int label) {
        try {
            if (isClassification) {
                ScipionMetaData.EMObject emo = ((ScipionMetaData) md).getEMObject(objId);
                return String.format("Class %s (%d images)", emo.getId(), emo.childmd.size());
            } else {
                return md.getValueString(label, objId);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public void readMd() {
        hasMdChanges = false;
        hasClassesChanges = false;
        md = getMetaData(selectedBlock);
    }

    public MetaData getMetaData(String block) {
        if (md.getBlock().equals(block)) {
            return md;
        }
        ScipionMetaData child = ((ScipionMetaData) md).getChild(block);
        if (child != null) {
            return child;
        }
        ScipionMetaData parent = ((ScipionMetaData) md).getParent();
        if (parent.getBlock().equals(selectedBlock))// from child to parent
        {
            return parent;
        }
        return parent.getChild(selectedBlock);

    }

    /**
     * Get the assigned class of some element
     */
    public ClassInfo getItemClassInfo(int index) {
        return null;
    }

    /**
     * Set item class info in md
     */
    private void setItemClassInfo(long id, ClassInfo cli) {

    }

    /**
     * Set the class of an element
     */
    public void setItemClass(int index, ClassInfo cli) {

    }

    public ClassInfo getClassInfo(int classNumber) {
        return null;
    }

    /**
     * Compute and update the number of classes and images assigned to this
     * superclass
     */
    public void updateClassesInfo() {

    }// function upateClassesInfo

    /**
     * Load classes structure if previously stored
     */
    public void loadClassesInfo() {

    }// function loadClassesInfo

    public MetaData[] getClassesMd() {
        return null;
    }

    /**
     * Add a new class
     */
    public void addClass(ClassInfo ci) {

    }

    /**
     * Remove a class from the selection
     */
    public void removeClass(int classNumber) {

    }

    public boolean hasClasses()//for Scipion usage only
    {
        return mdBlocks.length > 1 && ((ScipionMetaData) md).getSelf().contains("Class");
    }

    public boolean hasMicrographParticles() {
        return false;//fixme?? cannot open picker from sqlite
    }

    public List<ScipionMetaData.EMObject> getEMObjects() {
        return ((ScipionMetaData)md).getEMObjects();
    }

    

    /**
     * This is only needed for metadata table galleries
     */
    public boolean isFile(ColumnInfo ci) {
        return ci.labelName.contains("filename");
    }

    public boolean isImageFile(ColumnInfo ci) {
        return ci.allowRender;
    }

    public MetaData getMd(List<Long> ids) {
        MetaData selmd = null;
        try {
            long[] ids2 = new long[ids.size()];
            for (int i = 0; i < ids.size(); i++) {
                ids2[i] = ids.get(i);
            }
            selmd = ((ScipionMetaData) md).getStructure("");
            selmd.importObjects(md, ids2);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return selmd;
    }

    

    public void overwrite(String path) throws SQLException {
        ((ScipionMetaData) md).overwrite(filename, path);
    }

    public String getScipionType() {
        if (hasClasses()) {
            return "Particle";
        }
        String self = ((ScipionMetaData) md).getSelf();
        
        return self;
    }

    public String getSelf() {

        return ((ScipionMetaData) md).getSelf();
    }

    public String getPreffix() {
        return ((ScipionMetaData) md).getPreffix();
    }

    public void exportCTFRecalculate(String path) {

        try {
            FileWriter fstream = new FileWriter(path);
            BufferedWriter out = new BufferedWriter(fstream);

            String format = "%10s%10.2f%10.2f%10.2f%10.2f%10.2f\n", line;
            EllipseCTF ctf;
            for (Map.Entry<Long,EllipseCTF> entry : ctfs.entrySet()) 
            {
                
                ctf = entry.getValue();
                line = String.format(Locale.ENGLISH, format, entry.getKey(), ctf.getDefocusU(), ctf.getDefocusV(), ctf.getEllipseFitter().angle, ctf.getLowFreq(), ctf.getHighFreq());
                out.write(line);
            }

            out.close();

        } catch (Exception ex) {
            ex.printStackTrace();
            XmippDialog.showError(window, ex.getMessage());
        }
    }
    
    @Override
    public void removeCTF(int row) {
        ScipionMetaData.EMObject emo = ((ScipionMetaData) md).getEMObjects().get(row);
        emo.setComment("");
        super.removeCTF(row);
        window.fireTableRowsUpdated(row, row);
    }

    

    public String createSortFile(String psdFile, int row) {
        return null;
    }
    
    
    public void recalculateCTF(int row, EllipseCTF ellipseCTF, String sortFn) {
        if(isEnabled(row))
        {
            if (ctfs == null) {
                ctfs = new HashMap<Long, EllipseCTF>();
            }

            ScipionMetaData.EMObject emo;
            for(int i = selfrom; i <= selto; i ++)
                if(selection[i] && isEnabled(i))
                {
                    emo = ((ScipionMetaData) md).getEMObjects().get(i);
                    emo.setComment("(recalculate ctf)");
                    ctfs.put(ids[i], ellipseCTF);
                }
            window.fireTableRowsUpdated(selfrom, selto);
        }
    }
    
    
    public Geometry getGeometry(long id)
        {
            if (!containsGeometryInfo()) //FIXME: Now not reading any geometry!!!
                return null;
            ScipionMetaData.EMObject emo = ((ScipionMetaData)md).getEMObject(id);
            Double shiftx, shifty, psiangle;
            shiftx = emo.getValueDouble("_alignment._xmipp_shiftX");
            shifty = emo.getValueDouble("_alignment._xmipp_shiftY");
            psiangle =  emo.getValueDouble("_alignment._xmipp_anglePsi");
            Boolean flip = emo.getValueBoolean("_alignment._xmipp_flip") ;
            return new Geometry(shiftx, shifty, psiangle, flip);
        }

    public int getEnabledCount() {
        return ((ScipionMetaData)md).getEnabledCount();
    }
        
     /**
     * Set enabled state
     */
    @Override
    public void setEnabled(int index, boolean isenabled) {
        try {
            if (!isVolumeMode()) { // slices in a volume are always enabled
                getEMObjects().get(index).setEnabled(isenabled);
                hasMdChanges = true;
                if(!isenabled && isRecalculateCTF(index))
                    removeCTF(index);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public boolean hasMdChanges()
    {
        return ((ScipionMetaData)md).isChanged();
    }
    
    /**
     * Check if an item is enabled or not
     */
    public boolean isEnabled(int index) {
        try {
            if (isVolumeMode()) {
                return true;
            }
            return getEMObjects().get(index).isEnabled();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return true;
    }
    
    public String getValueFromCol(int index, ColumnInfo ci) {
        try {
            return getEMObjects().get(index).getValueString(ci);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

     public void setValueToCol(int index, ColumnInfo ci, String value) {
        try {
            getEMObjects().get(index).setValue(ci, value);
            setMdChanges(true);
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
