/***************************************************************************
 * Copyright (C) 2026 by Mustafa Comoglu                                    *
 *                                                                          *
 * GNU Affero General Public License Usage                                  *
 * This file may be used under the terms of the GNU Affero                  *
 * Public License version 3.0 as published by the Free Software Foundation  *
 * and appearing in the file LICENSE included in the packaging of this      *
 * file. Please review the following information to ensure the GNU Affero   *
 * Public License version 3.0 requirements will be met:                     *
 * https://www.gnu.org/licenses/agpl-3.0.html.                              *
 ***************************************************************************/


#ifndef SEISCOMP_GUI_SELECTIONCRITERIADIALOG_H
#define SEISCOMP_GUI_SELECTIONCRITERIADIALOG_H


#include <seiscomp/gui/qt.h>

#include <QDialog>
#include <QHash>
#include <QList>
#include <QString>


class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QPushButton;


namespace Seiscomp {
namespace Gui {


/**
 * @brief Reusable dialog to select table rows by one or more numeric criteria.
 *
 * The dialog shows one row per criterion, each with an enable check box and
 * either a [min, max] range or a single upper-bound spin box, plus an action
 * selector that decides what happens to the matching / non-matching rows.
 *
 * The dialog is domain agnostic: the caller describes the criteria to offer
 * and afterwards evaluates the returned values against its own model. A row
 * "matches" when it satisfies *all* enabled criteria (logical AND).
 */
class SC_GUI_API SelectionCriteriaDialog : public QDialog {
	Q_OBJECT

	public:
		enum ApplyMode {
			//! Activate matching rows, deactivate all others
			ActivateMatchingDeactivateRest,
			//! Deactivate matching rows, leave the rest untouched
			DeactivateMatching,
			//! Deactivate non-matching rows, leave matching rows untouched
			DeactivateNonMatching
		};

		/**
		 * @brief Description of a single selectable criterion.
		 */
		struct Criterion {
			QString id;                //!< key used to look up the result
			QString label;             //!< text shown to the user
			QString unit;              //!< spin box suffix, may be empty
			double  minimum{0.0};      //!< spin box lower bound
			double  maximum{1.0};      //!< spin box upper bound
			int     decimals{2};       //!< spin box precision
			bool    range{true};       //!< true: [low, high]; false: single bound
			double  defaultLow{0.0};   //!< initial value of the low spin box
			double  defaultHigh{1.0};  //!< initial value of the high spin box
			bool    enabledByDefault{false};
		};

		/**
		 * @brief Values entered by the user, keyed by Criterion::id.
		 *
		 * For a single-bound criterion (Criterion::range == false) only
		 * high() carries a meaningful value; low() returns Criterion::minimum.
		 */
		class Result {
			public:
				bool enabled(const QString &id) const;
				double low(const QString &id) const;
				double high(const QString &id) const;
				ApplyMode mode() const { return _mode; }

				//! At least one criterion is enabled.
				bool anyEnabled() const { return !_values.isEmpty(); }

			private:
				struct Value {
					double low{0.0};
					double high{0.0};
				};

				QHash<QString, Value> _values;
				ApplyMode             _mode{ActivateMatchingDeactivateRest};

			friend class SelectionCriteriaDialog;
		};

	public:
		/**
		 * @param title      window title
		 * @param objectNoun plural noun for the affected rows, e.g. "arrivals";
		 *                   used in the explanatory text and action labels
		 * @param criteria   criteria to offer, in display order
		 */
		SelectionCriteriaDialog(const QString &title,
		                        const QString &objectNoun,
		                        const QList<Criterion> &criteria,
		                        QWidget *parent = nullptr);

		//! Returns the values as entered. Only valid after exec() == Accepted.
		Result result() const;

	private:
		void updateEnabledState();

	private:
		struct Row {
			Criterion       def;
			QCheckBox      *enable{nullptr};
			QDoubleSpinBox *low{nullptr};
			QDoubleSpinBox *high{nullptr};
		};

		QList<Row>   _rows;
		QComboBox   *_action{nullptr};
		QPushButton *_okButton{nullptr};
};


} // namespace Gui
} // namespace Seiscomp


#endif
