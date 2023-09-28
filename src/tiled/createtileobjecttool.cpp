/*
 * createtileobjecttool.cpp
 * Copyright 2014, Martin Ziel <martin.ziel.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "createtileobjecttool.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "snaphelper.h"
#include "tile.h"
#include "utils.h"

using namespace Tiled;

CreateTileObjectTool::CreateTileObjectTool(QObject *parent)
    : CreateObjectTool("CreateTileObjectTool", parent)
{
    QIcon icon(QLatin1String(":images/24/insert-image.png"));
    icon.addFile(QLatin1String(":images/48/insert-image.png"));
    setIcon(icon);
    setShortcut(Qt::Key_T);
    Utils::setThemeIcon(this, "insert-image");
    languageChangedImpl();
}

void CreateTileObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    MapObject *newMapObject = mNewMapObjectItem->mapObject();

    if (state() == Preview && tile() && mCell.tile() != tile()) {
        setCell(Cell(tile()));
        mRotation = 0;

        newMapObject->setCell(mCell);
        newMapObject->setRotation(mRotation);
        newMapObject->setSize(tile()->size());
        mNewMapObjectItem->update();
        mNewMapObjectItem->syncWithMapObject();
    }

    // todo: take into account rotation when positioning the preview

    const QSize imgSize = newMapObject->cell().tile()->size();
    const QPointF halfSize(imgSize.width() / 2, imgSize.height() / 2);
    const QRectF screenBounds { pos - halfSize, imgSize };

    // These screenBounds assume TopLeft alignment, but the map's object alignment might be different.
    const QPointF offset = alignmentOffset(screenBounds, newMapObject->alignment(mapDocument()->map()));

    const MapRenderer *renderer = mapDocument()->renderer();
    QPointF pixelCoords = renderer->screenToPixelCoords(screenBounds.topLeft() + offset);

    SnapHelper(renderer, modifiers).snap(pixelCoords);

    newMapObject->setPosition(pixelCoords);
    mNewMapObjectItem->syncWithMapObject();
}

void CreateTileObjectTool::languageChanged()
{
    CreateObjectTool::languageChanged();
    languageChangedImpl();
}

void CreateTileObjectTool::languageChangedImpl()
{
    setName(tr("Insert Tile"));
}

MapObject *CreateTileObjectTool::createNewMapObject()
{
    if (!tile())
        return nullptr;

    if (mCell.tile() != tile()) {
        setCell(Cell(tile()));
        mRotation = 0;
    }

    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Rectangle);
    newMapObject->setCell(mCell);
    newMapObject->setSize(tile()->size());
    newMapObject->setRotation(mRotation);
    return newMapObject;
}

void CreateTileObjectTool::flipHorizontally()
{
    mCell.setFlippedHorizontally(!mCell.flippedHorizontally());

    switch (state()) {
    case Idle:
        break;
    case Preview:
    case CreatingObject: {
        MapObject *newMapObject = mNewMapObjectItem->mapObject();
        newMapObject->setCell(mCell);
        mNewMapObjectItem->update();
        break;
    }
    }
}

void CreateTileObjectTool::flipVertically()
{
    mCell.setFlippedVertically(!mCell.flippedVertically());

    switch (state()) {
    case Idle:
        break;
    case Preview:
    case CreatingObject: {
        MapObject *newMapObject = mNewMapObjectItem->mapObject();
        newMapObject->setCell(mCell);
        mNewMapObjectItem->update();
        break;
    }
    }
}

void CreateTileObjectTool::rotateLeft()
{
    mRotation -= 90;
    if (mRotation < -180)
        mRotation += 360;

    switch (state()) {
    case Idle:
        break;
    case Preview:
    case CreatingObject: {
        MapObject *newMapObject = mNewMapObjectItem->mapObject();
        newMapObject->setRotation(mRotation);
        mNewMapObjectItem->syncWithMapObject();
        break;
    }
    }
}

void CreateTileObjectTool::rotateRight()
{
    mRotation += 90;
    if (mRotation > 180)
        mRotation -= 360;

    switch (state()) {
    case Idle:
        break;
    case Preview:
    case CreatingObject: {
        MapObject *newMapObject = mNewMapObjectItem->mapObject();
        newMapObject->setRotation(mRotation);
        mNewMapObjectItem->syncWithMapObject();
        break;
    }
    }
}

void CreateTileObjectTool::setCell(const Cell &cell)
{
    mCell = cell;
    mTileset = cell.tileset() ? cell.tileset()->sharedFromThis()
                              : SharedTileset();
}

/// EDEN CHANGES
float Randomf(float range)
{
	const float OneOver = 1.f / RAND_MAX;
	return (range * OneOver * static_cast<float>(rand() % RAND_MAX));
}

float Randomf(float min, float max)
{
	const float range = (max - min);
	return min + Randomf(range);
}

void CreateTileObjectTool::copySpecificProperties(MapObject* newMapObject, Tile* pTile)
{
  for (QMap<QString, QVariant>::const_iterator i = pTile->properties().constBegin(); i != pTile->properties().constEnd(); ++i)
  {
    QString property = i.key();
    QString value = i.value().toString();

    if (property.contains(QStringLiteral("Wavy"))
      || property == QStringLiteral("Name")
      || property == QStringLiteral("Type")
      || property == QStringLiteral("RandomizedProp"))
    {
      newMapObject->setProperty(property, value);
    }
  }
}

void CreateTileObjectTool::copySpecificPropertiesFromObject(MapObject* newMapObject, const MapObject* pSourceObject)
{
  for (QMap<QString, QVariant>::const_iterator i = pSourceObject->properties().constBegin(); i != pSourceObject->properties().constEnd(); ++i)
  {
    QString property = i.key();
    QString value = i.value().toString();

    if (newMapObject->IsPropertyRandomized(property) == false || property == QStringLiteral("RandomizedProp"))
    {
      newMapObject->setProperty(property, value);
    }
  }
}

void CreateTileObjectTool::randomizeProperties(MapObject* newMapObject, Tile* pTile, int iObjectIDIncrement, bool bClearRandomizedProperties)
{
	int iNumRandomObjects = -1;
	float fRandomNewObjectsXMin = -1.0f;
	float fRandomNewObjectsXMax = -1.0f;
	float fRandomNewObjectsYMin = -1.0f;
	float fRandomNewObjectsYMax = -1.0f;

	int iObjectID = 0;

	if (bClearRandomizedProperties)
	{
		for (QMap<QString, QVariant>::const_iterator i = newMapObject->properties().constBegin(); i != newMapObject->properties().constEnd(); )
		{
			QString property = i.key();
			QString value = i.value().toString();

			if (newMapObject->IsPropertyRandomized(property))
			{
				newMapObject->removeProperty(property);
				i = newMapObject->properties().constBegin();
			}
			else
			{
				++i;
			}
		}
	}

	for (QMap<QString, QVariant>::const_iterator i = pTile->properties().constBegin(); i != pTile->properties().constEnd(); ++i)
	{
		QString property = i.key();
		QString value = i.value().toString();

		if (property.contains(QStringLiteral("RandomRotation")))
		{
			QString rotation = i.value().toString();
			QString comma = QStringLiteral(",");
			QStringRef minRotation(&rotation.data()[0], &rotation.data()[rotation.indexOf(comma)]);
			QStringRef maxRotation(&rotation.data()[rotation.indexOf(comma) + 1], &rotation.data()[rotation.length() - rotation.indexOf(comma) - 1]);

			QString minRot;
			minRot += minRotation;

			QString maxRot;
			maxRot += maxRotation;

			int iMin = minRot.toInt();
			int iMax = maxRot.toInt();

			QString value;
			//+1 so we have a chance of hitting the max
			value.setNum(rand() % (iMax - iMin + 1) + iMin);

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, value);
		}
		else if (property.contains(QStringLiteral("RotationSet")) || property.contains(QStringLiteral("SetRotation")))
		{
			QString rotation = i.value().toString();
			QStringList rotationList = rotation.split(QStringLiteral(","));

			QString value;
			value.setNum(rotationList[rand() % rotationList.size()].toInt());

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, value);
		}
		else if (property.contains(QStringLiteral("RandomHueToSaturationLink")))
		{
			QString alpha = i.value().toString();
			QString comma = QStringLiteral(",");

			QStringRef minalpha;
			QString alphaValue;

			if (alpha.indexOf(comma) >= 0)
			{
				minalpha = QStringRef(&alpha.data()[0], &alpha.data()[alpha.indexOf(comma)]);
				QStringRef maxalpha(&alpha.data()[alpha.indexOf(comma) + 1], &alpha.data()[alpha.length() - alpha.indexOf(comma) - 1]);

				QString minAlpha;
				minAlpha += minalpha;

				QString maxAlpha;
				maxAlpha += maxalpha;

				float fMin = minAlpha.toFloat();
				float fMax = maxAlpha.toFloat();
				alphaValue.setNum(Randomf(fMin, fMax));
			}
			else
			{
				minalpha = QStringRef(&alpha.data()[0], &alpha.data()[alpha.length()]);
				alphaValue += minalpha;
			}

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, alphaValue);
		}
		else if (property.contains(QStringLiteral("HueToSaturationLinkSet")))
		{
			QString alpha = i.value().toString();
			QStringList alphaList = alpha.split(QStringLiteral(","));

			QString alphaValue = alphaList[rand() % alphaList.size()];

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, alphaValue);
		}
		else if (property.contains(QStringLiteral("RandomScaleToHueLink")))
		{
			QString alpha = i.value().toString();
			QString comma = QStringLiteral(",");
			QStringRef minalpha;
			QString alphaValue;

			if (alpha.indexOf(comma) >= 0)
			{
				minalpha = QStringRef(&alpha.data()[0], &alpha.data()[alpha.indexOf(comma)]);
				QStringRef maxalpha(&alpha.data()[alpha.indexOf(comma) + 1], &alpha.data()[alpha.length() - alpha.indexOf(comma) - 1]);

				QString minAlpha;
				minAlpha += minalpha;

				QString maxAlpha;
				maxAlpha += maxalpha;

				float fMin = minAlpha.toFloat();
				float fMax = maxAlpha.toFloat();
				alphaValue.setNum(Randomf(fMin, fMax));
			}
			else
			{
				minalpha = QStringRef(&alpha.data()[0], &alpha.data()[alpha.length()]);
				alphaValue += minalpha;
			}

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, alphaValue);
		}
		else if (property.contains(QStringLiteral("ScaleToHueLinkSet")))
		{
			QString alpha = i.value().toString();
			QStringList alphaList = alpha.split(QStringLiteral(","));

			QString alphaValue = alphaList[rand() % alphaList.size()];

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, alphaValue);
		}
		else if (property.contains(QStringLiteral("RandomScale")))
		{
			QString scale = i.value().toString();
			QString comma = QStringLiteral(",");
			QStringRef minScale(&scale.data()[0], &scale.data()[scale.indexOf(comma)]);
			QStringRef maxScale(&scale.data()[scale.indexOf(comma) + 1], &scale.data()[scale.length() - scale.indexOf(comma) - 1]);

			QString minSca;
			minSca += minScale;

			QString maxSca;
			maxSca += maxScale;

			float fMin = minSca.toFloat();
			float fMax = maxSca.toFloat();

			float fRandom = Randomf(fMin, fMax);

			QString value;
			value.setNum(fRandom);

			if (property == QStringLiteral("RandomScaleAll"))
			{
				newMapObject->setWidth(newMapObject->width() * fRandom);
				newMapObject->setHeight(newMapObject->height() * fRandom);
			}

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, value);
		}
		else if (property.contains(QStringLiteral("ScaleSet")) || property.contains(QStringLiteral("SetScale")))
		{
			QString scale = i.value().toString();
			QStringList scaleList = scale.split(QStringLiteral(","));

			float fRandom = scaleList[rand() % scaleList.size()].toFloat();

			QString value;
			value.setNum(fRandom);

			if (property == QStringLiteral("ScaleSetAll") || property == QStringLiteral("SetScaleAll"))
			{
				newMapObject->setWidth(newMapObject->width() * fRandom);
				newMapObject->setHeight(newMapObject->height() * fRandom);
			}

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, value);
		}
		else if (property.contains(QStringLiteral("RandomXOffset")))
		{
			QString offset = i.value().toString();
			QString comma = QStringLiteral(",");
			QStringRef minOffset(&offset.data()[0], &offset.data()[offset.indexOf(comma)]);
			QStringRef maxOffset(&offset.data()[offset.indexOf(comma) + 1], &offset.data()[offset.length() - offset.indexOf(comma) - 1]);

			QString minOff;
			minOff += minOffset;

			QString maxOff;
			maxOff += maxOffset;

			float fMin = minOff.toFloat();
			float fMax = maxOff.toFloat();

			float fRandom = Randomf(fMin, fMax);

			QString value;
			value.setNum(fRandom);

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, value);
		}
		else if (property.contains(QStringLiteral("RandomYOffset")))
		{
			QString offset = i.value().toString();
			QString comma = QStringLiteral(",");
			QStringRef minOffset(&offset.data()[0], &offset.data()[offset.indexOf(comma)]);
			QStringRef maxOffset(&offset.data()[offset.indexOf(comma) + 1], &offset.data()[offset.length() - offset.indexOf(comma) - 1]);

			QString minOff;
			minOff += minOffset;

			QString maxOff;
			maxOff += maxOffset;

			float fMin = minOff.toFloat();
			float fMax = maxOff.toFloat();

			float fRandom = Randomf(fMin, fMax);

			QString value;
			value.setNum(fRandom);

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, value);
		}
		else if (property.contains(QStringLiteral("RandomHue")))
		{
			QString hue = i.value().toString();
			QString comma = QStringLiteral(",");
			QStringRef minhue(&hue.data()[0], &hue.data()[hue.indexOf(comma)]);
			QStringRef maxhue(&hue.data()[hue.indexOf(comma) + 1], &hue.data()[hue.length() - hue.indexOf(comma) - 1]);

			QString minHue;
			minHue += minhue;

			QString maxHue;
			maxHue += maxhue;

			float fMin = minHue.toFloat();
			float fMax = maxHue.toFloat();

			QString hueValue;
			hueValue.setNum(Randomf(fMin, fMax));

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, hueValue);
		}
		else if (property.contains(QStringLiteral("HueSet")) || property.contains(QStringLiteral("SetHue")))
		{
			QString hue = i.value().toString();
			QStringList hueList = hue.split(QStringLiteral(","));

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, hueList[rand() % hueList.size()]);
		}
		else if (property.contains(QStringLiteral("RandomDarken")))
		{
			QString darken = i.value().toString();
			QString comma = QStringLiteral(",");
			QStringRef mindarken(&darken.data()[0], &darken.data()[darken.indexOf(comma)]);
			QStringRef maxdarken(&darken.data()[darken.indexOf(comma) + 1], &darken.data()[darken.length() - darken.indexOf(comma) - 1]);

			QString minDarken;
			minDarken += mindarken;

			QString maxDarken;
			maxDarken += maxdarken;

			float fMin = minDarken.toFloat();
			float fMax = maxDarken.toFloat();

			QString darkenValue;
			darkenValue.setNum(Randomf(fMin, fMax));

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, darkenValue);
		}
		else if (property.contains(QStringLiteral("DarkenSet")) || property.contains(QStringLiteral("SetDarken")))
		{
			QString darken = i.value().toString();
			QStringList darkenList = darken.split(QStringLiteral(","));

			QString darkenValue = darkenList[rand() % darkenList.size()];

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, darkenValue);
		}
		else if (property.contains(QStringLiteral("RandomOpacity")))
		{
			QString alpha = i.value().toString();
			QString comma = QStringLiteral(",");
			QStringRef minalpha(&alpha.data()[0], &alpha.data()[alpha.indexOf(comma)]);
			QStringRef maxalpha(&alpha.data()[alpha.indexOf(comma) + 1], &alpha.data()[alpha.length() - alpha.indexOf(comma) - 1]);

			QString minAlpha;
			minAlpha += minalpha;

			QString maxAlpha;
			maxAlpha += maxalpha;

			float fMin = minAlpha.toFloat();
			float fMax = maxAlpha.toFloat();

			QString alphaValue;
			alphaValue.setNum(Randomf(fMin, fMax));

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, alphaValue);
		}
		else if (property.contains(QStringLiteral("OpacitySet")) || property.contains(QStringLiteral("SetOpacity")))
		{
			QString alpha = i.value().toString();
			QStringList alphaList = alpha.split(QStringLiteral(","));

			QString alphaValue = alphaList[rand() % alphaList.size()];

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, alphaValue);
		}
		else if (property.contains(QStringLiteral("RandomSaturation")))
		{
			QString alpha = i.value().toString();
			QString comma = QStringLiteral(",");
			QStringRef minalpha(&alpha.data()[0], &alpha.data()[alpha.indexOf(comma)]);
			QStringRef maxalpha(&alpha.data()[alpha.indexOf(comma) + 1], &alpha.data()[alpha.length() - alpha.indexOf(comma) - 1]);

			QString minAlpha;
			minAlpha += minalpha;

			QString maxAlpha;
			maxAlpha += maxalpha;

			float fMin = minAlpha.toFloat();
			float fMax = maxAlpha.toFloat();

			QString alphaValue;
			alphaValue.setNum(Randomf(fMin, fMax));

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, alphaValue);
		}
		else if (property.contains(QStringLiteral("SaturationSet")) || property.contains(QStringLiteral("SetSaturation")))
		{
			QString alpha = i.value().toString();
			QStringList alphaList = alpha.split(QStringLiteral(","));

			QString alphaValue = alphaList[rand() % alphaList.size()];

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, alphaValue);
		}
		else if (property.contains(QStringLiteral("RandomFlip")))
		{
			QString value;
			value.setNum(rand());

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, value);
		}
		else if (property.contains(QStringLiteral("RandomAnimation")))
		{
			QString value;
			value.setNum(rand() % 100);

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, value);
		}
		else if (property.contains(QStringLiteral("SubObjectChoice")))
		{
			QString value;
			value.setNum(rand());

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, value);
		}
		else if (property.contains(QStringLiteral("ComponentChoice")))
		{
			QString value;
			value.setNum(rand());

			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, value);
		}
		else if (property.contains(QStringLiteral("source")) && (i.value().toString().contains(QStringLiteral("(RAND)_"), Qt::CaseInsensitive) || i.value().toString().contains(QStringLiteral("{RAND}_"), Qt::CaseInsensitive)))
		{
			QString anim = i.value().toString();
			QString token = QStringLiteral("_");
			QStringRef path(&anim.data()[anim.indexOf(token) + 1], &anim.data()[anim.length() - anim.indexOf(token) - 1]);

			QString sPath;
			sPath += path;
			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, sPath);

			QString value;
			value.setNum(rand());

			token = QStringLiteral("source");
			QStringRef objectName(&property.data()[0], &property.data()[property.indexOf(token)]);
			QString sObjectProperty;
			sObjectProperty += objectName;
			sObjectProperty += QStringLiteral("RandomSourceIndex");

			newMapObject->setProperty(sObjectProperty, value);
		}
		else if (property.contains(QStringLiteral("RandomNumToSpawn")))
		{
			QString randomNum = i.value().toString();
			QString comma = QStringLiteral(",");

			// Not sure about this, but QStringRef does not seem to exist in 6.0 onwards...
			QStringRef minrandomNum(randomNum);
			QStringRef maxrandomNum(randomNum);

			QString minRandomNum;
			minRandomNum += minrandomNum;

			QString maxRandomNum;
			maxRandomNum += maxrandomNum;

			int iMin = minRandomNum.toInt();
			int iMax = maxRandomNum.toInt();

			newMapObject->removeProperty(property);

			QString token = QStringLiteral("RandomNumToSpawn");
			QStringRef objectName(property);

			{
				QString RandomNumToSpawnMin = QStringLiteral("RandomNumToSpawnMin");
				QString sObjectProperty;
				sObjectProperty += objectName;
				sObjectProperty.append(RandomNumToSpawnMin);
				newMapObject->setProperty(sObjectProperty, minRandomNum);
			}

			{
				QString RandomNumToSpawnMax = QStringLiteral("RandomNumToSpawnMax");
				QString sObjectProperty;
				sObjectProperty += objectName;
				sObjectProperty.append(RandomNumToSpawnMax);
				newMapObject->setProperty(sObjectProperty, maxRandomNum);
			}
		}
		else if (property.contains(QStringLiteral("RandomInclusion")))
		{
			int iRandom = rand() % 100;

			if (i.value().toInt() > iRandom)
			{
				newMapObject->setRandomized(true);
				newMapObject->setProperty(property, QStringLiteral("true"));
			}
			else
			{
				newMapObject->setRandomized(true);
				newMapObject->setProperty(property, QStringLiteral("false"));
			}
		}
		else if (property.contains(QStringLiteral("NumToSpawnSet")))
		{
			QString randomNum = i.value().toString();
			QStringList randomNumList = randomNum.split(QStringLiteral(","));

			iNumRandomObjects = randomNumList[rand() % randomNumList.size()].toInt();
		}
		else if (property.contains(QStringLiteral("RandomNewObjectsXOffset")))
		{
			newMapObject->setProperty(property, i.value());
		}
		else if (property.contains(QStringLiteral("RandomNewObjectsYOffset")))
		{
			newMapObject->setProperty(property, i.value());
		}
		else if (property.contains(QStringLiteral("source")) && (i.value().toString().contains(QStringLiteral("(RSA_"), Qt::CaseInsensitive) || i.value().toString().contains(QStringLiteral("{RSA_"), Qt::CaseInsensitive)))
		{
			QString anim = i.value().toString();
			QString token = QStringLiteral(")_");
			QStringRef path(&anim.data()[anim.indexOf(token, 5) + 2], &anim.data()[anim.length() - anim.indexOf(token, 5) - 2]);

			QString sPath;
			sPath += path;
			newMapObject->setRandomized(true);
			newMapObject->setProperty(property, sPath);

			QString value;

			token = QStringLiteral("source");
			QStringRef objectName(&property.data()[0], &property.data()[property.indexOf(token)]);
			QString sObjectProperty;
			QString RandomSourceIndex = QStringLiteral("RandomSourceIndex");
			sObjectProperty += property;
			sObjectProperty.replace(token, RandomSourceIndex);

			value.setNum(rand());

			newMapObject->setProperty(sObjectProperty, value);

			//newMapObject->removeProperty(property);

			//Since we might scale the object, handle in LevelBuilder
			/*
			for (int iCurrRandomNum = 0; iCurrRandomNum < iNumRandomObjects; ++iCurrRandomNum)
			{
				QString sNewSubObjectName;
				QString sNewSubObjectProperty;
				sNewSubObjectProperty += QStringLiteral("NewSubobject-");
				sNewSubObjectName += objectName;
				value.setNum(iObjectID);
				sNewSubObjectName += value;
				sNewSubObjectProperty += sNewSubObjectName;

				newMapObject->setProperty(sNewSubObjectProperty, sPath);

				value.setNum(rand());

				sNewSubObjectProperty = sNewSubObjectName;
				sNewSubObjectProperty += QStringLiteral("RandomSourceIndex");

				newMapObject->setProperty(sNewSubObjectProperty, value);

				sNewSubObjectProperty = sNewSubObjectName;
				sNewSubObjectProperty += QStringLiteral("RandomXOffset");

				float fRandom = Randomf(fRandomNewObjectsXMin, fRandomNewObjectsXMax);
				value.setNum(fRandom);

				newMapObject->setProperty(sNewSubObjectProperty, value);

				sNewSubObjectProperty = sNewSubObjectName;
				sNewSubObjectProperty += QStringLiteral("RandomYOffset");

				fRandom = Randomf(fRandomNewObjectsYMin, fRandomNewObjectsYMax);
				value.setNum(fRandom);

				newMapObject->setProperty(sNewSubObjectProperty, value);

				sNewSubObjectProperty = sNewSubObjectName;
				sNewSubObjectProperty += QStringLiteral("source");

				newMapObject->setProperty(sNewSubObjectProperty, sPath);

				++iObjectID;
			}

			iNumRandomObjects = -1;
			fRandomNewObjectsXMin = -1.0f;
			fRandomNewObjectsXMax = -1.0f;
			fRandomNewObjectsYMin = -1.0f;
			fRandomNewObjectsYMax = -1.0f;*/
		}
	}

	if (newMapObject->hasProperty(QStringLiteral("Type")))
	{
		newMapObject->setType(newMapObject->property(QStringLiteral("Type")).toString());
		newMapObject->removeProperty(QStringLiteral("Type"));
	}

	if (newMapObject->hasProperty(QStringLiteral("Name")))
	{
		QString filename = mapDocument()->fileName();

		if (filename.indexOf(QStringLiteral("data/Levels")) >= 0)
		{
			QStringRef path(&filename.data()[mapDocument()->fileName().indexOf(QStringLiteral("data/Levels"), 0, Qt::CaseInsensitive) + 12], &filename.data()[mapDocument()->fileName().length() - mapDocument()->fileName().indexOf(QStringLiteral("data/Levels"), 0, Qt::CaseInsensitive) - 12]);

			QString sPath;
			sPath += path;

			QString name = newMapObject->property(QStringLiteral("Name")).toString() + QString::number(mapDocument()->map()->nextObjectId() + iObjectIDIncrement) + QStringLiteral("_") + sPath;
			newMapObject->setName(name);
		}
		else
		{
			QStringRef path(&filename.data()[mapDocument()->fileName().indexOf(QStringLiteral("data\\Levels"), 0, Qt::CaseInsensitive) + 12], &filename.data()[mapDocument()->fileName().length() - mapDocument()->fileName().indexOf(QStringLiteral("data\\Levels"), 0, Qt::CaseInsensitive) - 12]);

			QString sPath;
			sPath += path;

			QString name = newMapObject->property(QStringLiteral("Name")).toString() + QString::number(mapDocument()->map()->nextObjectId() + iObjectIDIncrement) + QStringLiteral("_") + sPath;
			newMapObject->setName(name);
		}

		newMapObject->removeProperty(QStringLiteral("Name"));
	}

	return;
}
/// EDEN CHANGES END

#include "moc_createtileobjecttool.cpp"
