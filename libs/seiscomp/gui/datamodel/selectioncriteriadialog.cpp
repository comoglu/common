/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/


#include <seiscomp/gui/datamodel/selectioncriteriadialog.h>
#include <seiscomp/gui/core/application.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>


namespace Seiscomp {
namespace Gui {


bool SelectionCriteriaDialog::Result::enabled(const QString &id) const {
	return _values.contains(id);
}


double SelectionCriteriaDialog::Result::low(const QString &id) const {
	auto it = _values.find(id);
	return it != _values.end() ? it->low : 0.0;
}


double SelectionCriteriaDialog::Result::high(const QString &id) const {
	auto it = _values.find(id);
	return it != _values.end() ? it->high : 0.0;
}


SelectionCriteriaDialog::SelectionCriteriaDialog(const QString &title,
                                                 const QString &objectNoun,
                                                 const QList<Criterion> &criteria,
                                                 QWidget *parent)
: QDialog(parent) {
	setWindowTitle(title);

	auto *mainLayout = new QVBoxLayout(this);

	auto *info = new QLabel(
		tr("Enable one or more criteria below. %1 matching all enabled "
		   "criteria are treated according to the selected action.")
		.arg(objectNoun), this);
	info->setWordWrap(true);
	mainLayout->addWidget(info);

	auto *grid = new QGridLayout;
	grid->setColumnStretch(1, 1);
	grid->setColumnStretch(3, 1);

	int gridRow = 0;
	for ( const Criterion &def : criteria ) {
		Row row;
		row.def = def;

		row.enable = new QCheckBox(def.label, this);
		row.enable->setChecked(def.enabledByDefault);
		grid->addWidget(row.enable, gridRow, 0);

		row.high = new QDoubleSpinBox(this);
		row.high->setDecimals(def.decimals);
		row.high->setRange(def.minimum, def.maximum);
		row.high->setValue(def.defaultHigh);
		row.high->setAlignment(Qt::AlignRight);
		if ( !def.unit.isEmpty() )
			row.high->setSuffix(" " + def.unit);

		if ( def.range ) {
			row.low = new QDoubleSpinBox(this);
			row.low->setDecimals(def.decimals);
			row.low->setRange(def.minimum, def.maximum);
			row.low->setValue(def.defaultLow);
			row.low->setAlignment(Qt::AlignRight);
			if ( !def.unit.isEmpty() )
				row.low->setSuffix(" " + def.unit);

			grid->addWidget(row.low, gridRow, 1);
			grid->addWidget(new QLabel(tr("to"), this), gridRow, 2, Qt::AlignHCenter);
			grid->addWidget(row.high, gridRow, 3);
		}
		else {
			grid->addWidget(row.high, gridRow, 1, 1, 3);
		}

		connect(row.enable, &QCheckBox::toggled,
		        this, &SelectionCriteriaDialog::updateEnabledState);

		_rows.append(row);
		++gridRow;
	}

	mainLayout->addLayout(grid);

	auto *sep = new QFrame(this);
	sep->setFrameShape(QFrame::HLine);
	sep->setFrameShadow(QFrame::Sunken);
	mainLayout->addWidget(sep);

	auto *actionLayout = new QHBoxLayout;
	actionLayout->addWidget(new QLabel(tr("Action:"), this));
	_action = new QComboBox(this);
	_action->addItem(tr("Activate matching, deactivate the rest"),
	                 ActivateMatchingDeactivateRest);
	_action->addItem(tr("Deactivate matching"), DeactivateMatching);
	_action->addItem(tr("Deactivate non-matching"), DeactivateNonMatching);
	actionLayout->addWidget(_action, 1);
	mainLayout->addLayout(actionLayout);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
	                                     QDialogButtonBox::Cancel |
	                                     QDialogButtonBox::RestoreDefaults,
	                                     Qt::Horizontal, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(buttons->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
	        this, &SelectionCriteriaDialog::restoreDefaults);
	mainLayout->addWidget(buttons);

	_okButton = buttons->button(QDialogButtonBox::Ok);

	updateEnabledState();
}


void SelectionCriteriaDialog::updateEnabledState() {
	bool any = false;
	for ( const Row &row : _rows ) {
		bool on = row.enable->isChecked();
		if ( row.low )
			row.low->setEnabled(on);
		row.high->setEnabled(on);
		any = any || on;
	}

	if ( _okButton )
		_okButton->setEnabled(any);
}


void SelectionCriteriaDialog::restoreDefaults() {
	for ( const Row &row : _rows ) {
		row.enable->setChecked(row.def.enabledByDefault);
		if ( row.low )
			row.low->setValue(row.def.defaultLow);
		row.high->setValue(row.def.defaultHigh);
	}

	_action->setCurrentIndex(0);
	updateEnabledState();
}


void SelectionCriteriaDialog::setCriterionEnabled(const QString &id, bool enabled) {
	for ( const Row &row : _rows ) {
		if ( row.def.id == id ) {
			row.enable->setChecked(enabled);
			break;
		}
	}
}


void SelectionCriteriaDialog::loadSettings(const QString &groupName) {
	QSettings &s = SCApp->settings();
	s.beginGroup(groupName);

	for ( const Row &row : _rows ) {
		s.beginGroup(row.def.id);
		row.enable->setChecked(s.value("enabled", row.enable->isChecked()).toBool());
		// Values entered in another unit, e.g. km instead of degrees,
		// must not be reinterpreted
		if ( s.contains("unit") && s.value("unit").toString() == row.def.unit ) {
			if ( row.low )
				row.low->setValue(s.value("low", row.low->value()).toDouble());
			row.high->setValue(s.value("high", row.high->value()).toDouble());
		}
		s.endGroup();
	}

	int index = _action->findData(s.value("mode", _action->currentData()).toInt());
	if ( index >= 0 )
		_action->setCurrentIndex(index);

	s.endGroup();

	updateEnabledState();
}


void SelectionCriteriaDialog::saveSettings(const QString &groupName) const {
	QSettings &s = SCApp->settings();
	s.beginGroup(groupName);

	for ( const Row &row : _rows ) {
		s.beginGroup(row.def.id);
		s.setValue("enabled", row.enable->isChecked());
		s.setValue("unit", row.def.unit);
		if ( row.low )
			s.setValue("low", row.low->value());
		s.setValue("high", row.high->value());
		s.endGroup();
	}

	s.setValue("mode", _action->currentData().toInt());

	s.endGroup();
}


SelectionCriteriaDialog::Result SelectionCriteriaDialog::result() const {
	Result res;

	res._mode = static_cast<ApplyMode>(_action->currentData().toInt());

	for ( const Row &row : _rows ) {
		if ( !row.enable->isChecked() )
			continue;

		Result::Value v;
		v.low  = row.low ? row.low->value() : row.def.minimum;
		v.high = row.high->value();
		res._values.insert(row.def.id, v);
	}

	return res;
}


} // namespace Gui
} // namespace Seiscomp
